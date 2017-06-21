#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# =============================================================================
#  Version: 2.39 (September 29, 2015)
#  Author: Giuseppe Attardi (attardi@di.unipi.it), University of Pisa
#
#  Contributors:
#	Antonio Fuschetto (fuschett@aol.com)
#	Leonardo Souza (lsouza@amtera.com.br)
#	Juan Manuel Caicedo (juan@cavorite.com)
#	Humberto Pereira (begini@gmail.com)
#	Siegfried-A. Gevatter (siegfried@gevatter.com)
#	Pedro Assis (pedroh2306@gmail.com)
#	Wim Muskee (wimmuskee@gmail.com)
#	Radics Geza (radicsge@gmail.com)
#
# =============================================================================
#  Copyright (c) 2009-2015. Giuseppe Attardi (attardi@di.unipi.it).
# =============================================================================
#  This file is part of Tanl.
#
#  Tanl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License, version 3,
#  as published by the Free Software Foundation.
#
#  Tanl is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# =============================================================================

"""Wikipedia Extractor:
Extracts and cleans text from a Wikipedia database dump and stores output in a
number of files of similar size in a given directory.
Each file will contain several documents in the format:

	<doc id="" url="" title="">
        ...
        </doc>

This version performs template expansion by preprocesssng the whole dump and
collecting template definitions.
"""

import sys, os.path, time
import re                       # TODO use regex when it will be standard
import argparse, random
from itertools import izip,  izip_longest
import logging
import urllib
import bz2
import codecs
from htmlentitydefs import name2codepoint
from multiprocessing import Queue, JoinableQueue, Process, Manager, cpu_count
from cStringIO import StringIO
import fileinput
from timeit import default_timer

#===========================================================================

# Program version
version = '2.39'

### PARAMS ####################################################################

##
# Defined in <siteinfo>
# We include as default Template, when loading external template file.
knownNamespaces = set(['Template'])

##
# The namespace used for template definitions
# It is the name associated with namespace key=10 in the siteinfo header.
templateNamespace = ''

##
# The namespace used for module definitions
# It is the name associated with namespace key=828 in the siteinfo header.
moduleNamespace = ''

##
# Recognize only these namespaces
# w: Internal links to the Wikipedia
# wiktionary: Wiki dictionary
# wikt: shortcut for Wiktionary
#
acceptedNamespaces = ['w', 'wiktionary', 'wikt']

##
# Drop these elements from article text
#
discardElements = [
        'gallery', 'timeline', 'noinclude', 'pre',
        'table', 'tr', 'td', 'th', 'caption', 'div',
        'form', 'input', 'select', 'option', 'textarea',
        'ul', 'li', 'ol', 'dl', 'dt', 'dd', 'menu', 'dir',
        'ref', 'references', 'img', 'imagemap', 'source', 'small'
        ]

# This is obtained from <siteinfo>
urlbase = None

def get_url(id):
    global urlbase
    return "%s?curid=%s" % (urlbase, id)

#=========================================================================
#
# MediaWiki Markup Grammar
# https://www.mediawiki.org/wiki/Preprocessor_ABNF

# xml-char = %x9 / %xA / %xD / %x20-D7FF / %xE000-FFFD / %x10000-10FFFF
# sptab = SP / HTAB

# ; everything except ">" (%x3E)
# attr-char = %x9 / %xA / %xD / %x20-3D / %x3F-D7FF / %xE000-FFFD / %x10000-10FFFF

# literal         = *xml-char
# title           = wikitext-L3
# part-name       = wikitext-L3
# part-value      = wikitext-L3
# part            = ( part-name "=" part-value ) / ( part-value )
# parts           = [ title *( "|" part ) ]
# tplarg          = "{{{" parts "}}}"
# template        = "{{" parts "}}"
# link            = "[[" wikitext-L3 "]]"

# comment         = "<!--" literal "-->"
# unclosed-comment = "<!--" literal END
# ; the + in the line-eating-comment rule was absent between MW 1.12 and MW 1.22
# line-eating-comment = LF LINE-START *SP +( comment *SP ) LINE-END

# attr            = *attr-char
# nowiki-element  = "<nowiki" attr ( "/>" / ( ">" literal ( "</nowiki>" / END ) ) )

# wikitext-L2     = heading / wikitext-L3 / *wikitext-L2
# wikitext-L3     = literal / template / tplarg / link / comment /
#                   line-eating-comment / unclosed-comment / xmlish-element /
#                   *wikitext-L3

#------------------------------------------------------------------------------

selfClosingTags = [ 'br', 'hr', 'nobr', 'ref', 'references', 'nowiki' ]

# These tags are dropped, keeping their content.
# handle 'a' separately, depending on keepLinks
ignoredTags = [
    'abbr', 'b', 'big', 'blockquote', 'center', 'cite', 'div', 'em',
    'font', 'h1', 'h2', 'h3', 'h4', 'hiero', 'i', 'kbd', 'nowiki',
    'p', 'plaintext', 's', 'span', 'strike', 'strong',
    'sub', 'sup', 'tt', 'u', 'var'
]

placeholder_tags = {'math':'formula', 'code':'codice'}

def normalizeTitle(title):
    """Normalize title"""
    # remove leading/trailing whitespace and underscores
    title = title.strip(' _')
    # replace sequences of whitespace and underscore chars with a single space
    title = re.sub(r'[\s_]+', ' ', title)

    m = re.match(r'([^:]*):(\s*)(\S(?:.*))', title)
    if m:
        prefix = m.group(1)
        if m.group(2):
            optionalWhitespace = ' '
        else:
            optionalWhitespace = ''
        rest = m.group(3)

        ns = normalizeNamespace(prefix)
        if ns in knownNamespaces:
            # If the prefix designates a known namespace, then it might be
            # followed by optional whitespace that should be removed to get
            # the canonical page name
            # (e.g., "Category:  Births" should become "Category:Births").
            title = ns + ":" + ucfirst(rest)
        else:
            # No namespace, just capitalize first letter.
            # If the part before the colon is not a known namespace, then we
            # must not remove the space after the colon (if any), e.g.,
            # "3001: The_Final_Odyssey" != "3001:The_Final_Odyssey".
            # However, to get the canonical page name we must contract multiple
            # spaces into one, because
            # "3001:   The_Final_Odyssey" != "3001: The_Final_Odyssey".
            title = ucfirst(prefix) + ":" + optionalWhitespace + ucfirst(rest)
    else:
        # no namespace, just capitalize first letter
        title = ucfirst(title)
    return title

##
# Removes HTML or XML character references and entities from a text string.
#
# @param text The HTML (or XML) source text.
# @return The plain text, as a Unicode string, if necessary.

def unescape(text):
    def fixup(m):
        text = m.group(0)
        code = m.group(1)
        try:
            if text[1] == "#":  # character reference
                if text[2] == "x":
                    return unichr(int(code[1:], 16))
                else:
                    return unichr(int(code))
            else:               # named entity
                return unichr(name2codepoint[code])
        except:
            return text # leave as is

    return re.sub("&#?(\w+);", fixup, text)

# Match HTML comments
# The buggy template {{Template:T}} has a comment terminating with just "->"
comment = re.compile(r'<!--.*?-->', re.DOTALL)

# Match ignored tags
ignored_tag_patterns = []
def ignoreTag(tag):
    left = re.compile(r'<%s\b.*?>' % tag, re.IGNORECASE | re.DOTALL) # both <ref> and <reference>
    right = re.compile(r'</\s*%s>' % tag, re.IGNORECASE)
    ignored_tag_patterns.append((left, right))

for tag in ignoredTags:
    ignoreTag(tag)

# Match selfClosing HTML tags
selfClosing_tag_patterns = [
    re.compile(r'<\s*%s\b[^>]*/\s*>' % tag, re.DOTALL | re.IGNORECASE) for tag in selfClosingTags
]

# Match HTML placeholder tags
placeholder_tag_patterns = [
    (re.compile(r'<\s*%s(\s*| [^>]+?)>.*?<\s*/\s*%s\s*>' % (tag, tag), re.DOTALL | re.IGNORECASE),
     repl) for tag, repl in placeholder_tags.items()
]

# Match preformatted lines
preformatted = re.compile(r'^ .*?$')

# Match external links (space separates second optional parameter)
externalLink = re.compile(r'\[\w+[^ ]*? (.*?)]')
externalLinkNoAnchor = re.compile(r'\[\w+[&\]]*\]')

# Matches bold/italic
bold_italic = re.compile(r"'''''(.*?)'''''")
bold = re.compile(r"'''(.*?)'''")
italic_quote = re.compile(r"''\"([^\"]*?)\"''")
italic = re.compile(r"''(.*?)''")
quote_quote = re.compile(r'""([^"]*?)""')

# Matches space
spaces = re.compile(r' {2,}')

# Matches dots
dots = re.compile(r'\.{4,}')

#======================================================================

class Template(list):
    """
    A Template is a list of TemplateText or TemplateArgs
    """

    @classmethod
    def parse(cls, body):
        tpl = Template()
        # we must handle nesting, s.a.
        # {{{1|{{PAGENAME}}}
        # {{{italics|{{{italic|}}}
        # {{#if:{{{{{#if:{{{nominee|}}}|nominee|candidate}}|}}}|
        #
        start = 0
        for s,e in findMatchingBraces(body, 3):
            tpl.append(TemplateText(body[start:s]))
            tpl.append(TemplateArg(body[s+3:e-3]))
            start = e
        tpl.append(TemplateText(body[start:])) # leftover
        return tpl

    def subst(self, params, extractor, depth=0):
        # We perform parameter substitutions recursively.
        # We also limit the maximum number of iterations to avoid too long or
        # even endless loops (in case of malformed input).

        # :see: http://meta.wikimedia.org/wiki/Help:Expansion#Distinction_between_variables.2C_parser_functions.2C_and_templates
        #
        # Parameter values are assigned to parameters in two (?) passes.
        # Therefore a parameter name in a template can depend on the value of
        # another parameter of the same template, regardless of the order in
        # which they are specified in the template call, for example, using
        # Template:ppp containing "{{{{{{p}}}}}}", {{ppp|p=q|q=r}} and even
        # {{ppp|q=r|p=q}} gives r, but using Template:tvvv containing
        # "{{{{{{{{{p}}}}}}}}}", {{tvvv|p=q|q=r|r=s}} gives s.

        #logging.debug('subst tpl (%d, %d) %s', len(extractor.frame), depth, self)

        if depth > extractor.maxParameterRecursionLevels:
            extractor.recursion_exceeded_3_errs += 1
            return ''

        return ''.join([tpl.subst(params, extractor, depth) for tpl in self])

    def __str__(self):
        return ''.join([unicode(x) for x in self])

class TemplateText(unicode):
    """Fixed text of template"""

    def subst(self, params, extractor, depth):
        return self

class TemplateArg(object):
    """
    parameter to a template.
    Has a name and a default value, both of which are Templates.
    """
    def __init__(self, parameter):
        """
        :param parameter: the parts of a tplarg.
        """
        # the parameter name itself might contain templates, e.g.:
        #   appointe{{#if:{{{appointer14|}}}|r|d}}14|
        #   4|{{{{{subst|}}}CURRENTYEAR}}

        # any parts in a tplarg after the first (the parameter default) are
        # ignored, and an equals sign in the first part is treated as plain text.
        #logging.debug('TemplateArg %s', parameter)

        parts = splitParts(parameter)
        self.name = Template.parse(parts[0])
        if len(parts) > 1:
            # This parameter has a default value
            self.default = Template.parse(parts[1])
        else:
            self.default = None

    def __str__(self):
        if self.default:
            return '{{{%s|%s}}}' % (self.name, self.default)
        else:
            return '{{{%s}}}' % self.name

    def subst(self, params, extractor, depth):
        """
        Substitute value for this argument from dict :param params:
        Use :param extractor: to evaluate expressions for name and default.
        Limit substitution to the maximun :param depth:.
        """
        # the parameter name itself might contain templates, e.g.:
        # appointe{{#if:{{{appointer14|}}}|r|d}}14|
        paramName = self.name.subst(params, extractor, depth+1)
        paramName = extractor.expandTemplates(paramName)
        res = ''
        if paramName in params:
            res = params[paramName]  # use parameter value specified in template invocation
        elif self.default:            # use the default value
            defaultValue = self.default.subst(params, extractor, depth+1)
            res =  extractor.expandTemplates(defaultValue)
        #logging.debug('subst arg %d %s -> %s' % (depth, paramName, res))
        return res

#======================================================================

substWords = 'subst:|safesubst:'

class Extractor(object):
    """
    An extraction task on a article.
    """
    ##
    # Whether to preserve links in output
    keepLinks = False

    ##
    # Whether to transform sections into HTML
    keepSections = False

    ##
    # Whether to output HTML instead of text
    toHTML = False

    def __init__(self, id, title, page):
        """
        :param page: a list of lines.
        """
        self.id = id
        self.title = title
        self.page = page
        self.magicWords = MagicWords()
        self.frame = []
        self.recursion_exceeded_1_errs = 0 # template recursion within expandTemplates()
        self.recursion_exceeded_2_errs = 0 # template recursion within expandTemplate()
        self.recursion_exceeded_3_errs = 0 # parameter recursion
        self.template_title_errs = 0


    def extract(self, out):
        """
        :param out: a memory file.
        """
        logging.debug("%s\t%s", self.id, self.title)
        text = ''.join(self.page)
        url = get_url(self.id)
        header = '<doc id="%s" url="%s" title="%s">\n' % (self.id, url, self.title)
        # Separate header from text with a newline.
        header += self.title + '\n\n'
        header = header.encode('utf-8')
        self.magicWords['pagename'] = self.title
        self.magicWords['fullpagename'] = self.title
        self.magicWords['currentyear'] = time.strftime('%Y')
        self.magicWords['currentmonth'] = time.strftime('%m')
        self.magicWords['currentday'] = time.strftime('%d')
        self.magicWords['currenthour'] = time.strftime('%H')
        self.magicWords['currenttime'] = time.strftime('%H:%M:%S')
        text = clean(self, text)
        footer = "\n</doc>\n"
        out.write(header)
        for line in compact(text):
            out.write(line.encode('utf-8'))
            out.write('\n')
        out.write(footer)
        errs = (self.template_title_errs,
                self.recursion_exceeded_1_errs,
                self.recursion_exceeded_2_errs,
                self.recursion_exceeded_3_errs)
        if any(errs):
            logging.warn("Template errors in article '%s' (%s): title(%d) recursion(%d, %d, %d)",
                         self.title, self.id, *errs)

    #----------------------------------------------------------------------
    # Expand templates

    maxTemplateRecursionLevels = 30
    maxParameterRecursionLevels = 10

    # check for template beginning
    reOpen = re.compile('(?<!{){{(?!{)', re.DOTALL)

    def expandTemplates(self, wikitext):
        """
        :param wikitext: the text to be expanded.

        Templates are frequently nested. Occasionally, parsing mistakes may
        cause template insertion to enter an infinite loop, for instance when
        trying to instantiate Template:Country

        {{country_{{{1}}}|{{{2}}}|{{{2}}}|size={{{size|}}}|name={{{name|}}}}}

        which is repeatedly trying to insert template 'country_', which is
        again resolved to Template:Country. The straightforward solution of
        keeping track of templates that were already inserted for the current
        article would not work, because the same template may legally be used
        more than once, with different parameters in different parts of the
        article.  Therefore, we limit the number of iterations of nested
        template inclusion.

        """
        # Test template expansion at:
        # https://en.wikipedia.org/wiki/Special:ExpandTemplates

        res = ''
        if len(self.frame) >= self.maxTemplateRecursionLevels:
            self.recursion_exceeded_1_errs += 1
            return res

        #logging.debug('<expandTemplates ' + str(len(self.frame)))

        cur = 0
        # look for matching {{...}}
        for s,e in findMatchingBraces(wikitext, 2):
            res += wikitext[cur:s] + self.expandTemplate(wikitext[s+2:e-2])
            cur = e
        # leftover
        res += wikitext[cur:]
        #logging.debug('   expandTemplates> %d %s', len(self.frame), res)
        return res

    def templateParams(self, parameters):
        """
        Build a dictionary with positional or name key to expanded parameters.
        :param parameters: the parts[1:] of a template, i.e. all except the title.
        :param depth: recusion depth.
        """
        templateParams = {}

        if not parameters:
            return templateParams
        logging.debug('<templateParams: %s', '|'.join(parameters))

        # Parameters can be either named or unnamed. In the latter case, their
        # name is defined by their ordinal position (1, 2, 3, ...).

        unnamedParameterCounter = 0

        # It's legal for unnamed parameters to be skipped, in which case they
        # will get default values (if available) during actual instantiation.
        # That is {{template_name|a||c}} means parameter 1 gets
        # the value 'a', parameter 2 value is not defined, and parameter 3 gets
        # the value 'c'.  This case is correctly handled by function 'split',
        # and does not require any special handling.
        for param in parameters:
            # Spaces before or after a parameter value are normally ignored,
            # UNLESS the parameter contains a link (to prevent possible gluing
            # the link to the following text after template substitution)

            # Parameter values may contain "=" symbols, hence the parameter
            # name extends up to the first such symbol.

            # It is legal for a parameter to be specified several times, in
            # which case the last assignment takes precedence. Example:
            # "{{t|a|b|c|2=B}}" is equivalent to "{{t|a|B|c}}".
            # Therefore, we don't check if the parameter has been assigned a
            # value before, because anyway the last assignment should override
            # any previous ones.
            # FIXME: Don't use DOTALL here since parameters may be tags with
            # attributes, e.g. <div class="templatequotecite">
            # Parameters may span several lines, like:
            # {{Reflist|colwidth=30em|refs=
            # &lt;ref name=&quot;Goode&quot;&gt;Title&lt;/ref&gt;

            # The '=' might occurr within an HTML attribute:
            #   "&lt;ref name=value"
            # but we stop at first.
            m = re.match(' *([^=]*?) *=(.*)', param, re.DOTALL)
            if m:
                # This is a named parameter.  This case also handles parameter
                # assignments like "2=xxx", where the number of an unnamed
                # parameter ("2") is specified explicitly - this is handled
                # transparently.

                parameterName = m.group(1).strip()
                parameterValue = m.group(2)

                if ']]' not in parameterValue: # if the value does not contain a link, trim whitespace
                    parameterValue = parameterValue.strip()
                templateParams[parameterName] = parameterValue
            else:
                # this is an unnamed parameter
                unnamedParameterCounter += 1

                if ']]' not in param: # if the value does not contain a link, trim whitespace
                    param = param.strip()
                templateParams[str(unnamedParameterCounter)] = param
        logging.debug('   templateParams> %s', '|'.join(templateParams.values()))
        return templateParams

    def expandTemplate(self, body):
        """Expands template invocation.
        :param body: the parts of a template.

        :see http://meta.wikimedia.org/wiki/Help:Expansion for an explanation
        of the process.

        See in particular: Expansion of names and values
        http://meta.wikimedia.org/wiki/Help:Expansion#Expansion_of_names_and_values

        For most parser functions all names and values are expanded,
        regardless of what is relevant for the result. The branching functions
        (#if, #ifeq, #iferror, #ifexist, #ifexpr, #switch) are exceptions.

        All names in a template call are expanded, and the titles of the
        tplargs in the template body, after which it is determined which
        values must be expanded, and for which tplargs in the template body
        the first part (default).

        In the case of a tplarg, any parts beyond the first are never
        expanded.  The possible name and the value of the first part is
        expanded if the title does not match a name in the template call.

        :see code for braceSubstitution at
        https://doc.wikimedia.org/mediawiki-core/master/php/html/Parser_8php_source.html#3397:

        """

        # template        = "{{" parts "}}"

        # Templates and tplargs are decomposed in the same way, with pipes as
        # separator, even though eventually any parts in a tplarg after the first
        # (the parameter default) are ignored, and an equals sign in the first
        # part is treated as plain text.
        # Pipes inside inner templates and tplargs, or inside double rectangular
        # brackets within the template or tplargs are not taken into account in
        # this decomposition.
        # The first part is called title, the other parts are simply called parts.

        # If a part has one or more equals signs in it, the first equals sign
        # determines the division into name = value. Equals signs inside inner
        # templates and tplargs, or inside double rectangular brackets within the
        # part are not taken into account in this decomposition. Parts without
        # equals sign are indexed 1, 2, .., given as attribute in the <name> tag.

        if len(self.frame) >= self.maxTemplateRecursionLevels:
            self.recursion_exceeded_2_errs += 1
            #logging.debug('   INVOCATION> %d %s', len(self.frame), body)
            return ''

        logging.debug('INVOCATION %d %s', len(self.frame), body)

        parts = splitParts(body)
        # title is the portion before the first |
        logging.debug('TITLE %s', parts[0].strip())
        title = self.expandTemplates(parts[0].strip())

        # SUBST
        # Apply the template tag to parameters without
        # substituting into them, e.g.
        # {{subst:t|a{{{p|q}}}b}} gives the wikitext start-a{{{p|q}}}b-end
        # @see https://www.mediawiki.org/wiki/Manual:Substitution#Partial_substitution
        subst = False
        if re.match(substWords, title, re.IGNORECASE):
            title = re.sub(substWords, '', title, 1, re.IGNORECASE)
            subst = True

        if title.lower() in self.magicWords.values:
            return self.magicWords[title.lower()]

        # Parser functions
        # The first argument is everything after the first colon.
        # It has been evaluated above.
        colon = title.find(':')
        if colon > 1:
            funct = title[:colon]
            parts[0] = title[colon+1:].strip() # side-effect (parts[0] not used later)
            # arguments after first are not evaluated
            ret = callParserFunction(funct, parts, self.frame)
            return self.expandTemplates(ret)

        title = fullyQualifiedTemplateTitle(title)
        if not title:
            self.template_title_errs += 1
            return ''

        redirected = redirects.get(title)
        if redirected:
            title = redirected

        # get the template
        if title in templateCache:
            template = templateCache[title]
        elif title in templates:
            template = Template.parse(templates[title])
            # add it to cache
            templateCache[title] = template
            del templates[title]
        else:
            # The page being included could not be identified
            return ''

        #logging.debug('TEMPLATE %s: %s', title, template)

        # tplarg          = "{{{" parts "}}}"
        # parts           = [ title *( "|" part ) ]
        # part            = ( part-name "=" part-value ) / ( part-value )
        # part-name       = wikitext-L3
        # part-value      = wikitext-L3
        # wikitext-L3     = literal / template / tplarg / link / comment /
        #                   line-eating-comment / unclosed-comment /
        #		    xmlish-element / *wikitext-L3

        # A tplarg may contain other parameters as well as templates, e.g.:
        #   {{{text|{{{quote|{{{1|{{error|Error: No text given}}}}}}}}}}}
        # hence no simple RE like this would work:
        #   '{{{((?:(?!{{{).)*?)}}}'
        # We must use full CF parsing.

        # the parameter name itself might be computed, e.g.:
        #   {{{appointe{{#if:{{{appointer14|}}}|r|d}}14|}}}

        # Because of the multiple uses of double-brace and triple-brace
        # syntax, expressions can sometimes be ambiguous.
        # Precedence rules specifed here:
        # http://www.mediawiki.org/wiki/Preprocessor_ABNF#Ideal_precedence
        # resolve ambiguities like this:
        #   {{{{ }}}} -> { {{{ }}} }
        #   {{{{{ }}}}} -> {{ {{{ }}} }}
        #
        # :see: https://en.wikipedia.org/wiki/Help:Template#Handling_parameters

        params = parts[1:]

        if not subst:
            # Evaluate parameters, since they may contain templates, including
            # the symbol "=".
            # {{#ifexpr: {{{1}}} = 1 }}
            params = [self.expandTemplates(p) for p in params]

        # build a dict of name-values for the parameter values
        params = self.templateParams(params)

        # Perform parameter substitution
        # extend frame before subst, since there may be recursion in default
        # parameter value, e.g. {{OTRS|celebrative|date=April 2015}} in article
        # 21637542 in enwiki.
        self.frame.append((title, params))
        instantiated = template.subst(params, self)
        #logging.debug('instantiated %d %s', len(self.frame), instantiated)
        value = self.expandTemplates(instantiated)
        self.frame.pop()
        #logging.debug('   INVOCATION> %s %d %s', title, len(self.frame), value)
        return value

# ----------------------------------------------------------------------
# parameter handling

def splitParts(paramsList):
    """
    :param paramList: the parts of a template or tplarg.

    Split template parameters at the separator "|".
    separator "=".

    Template parameters often contain URLs, internal links, text or even
    template expressions, since we evaluate templates outside in.
    This is required for cases like:
      {{#if: {{{1}}} | {{lc:{{{1}}} | "parameter missing"}}
    Parameters are separated by "|" symbols. However, we
    cannot simply split the string on "|" symbols, since these
    also appear inside templates and internal links, e.g.

     {{if:|
      |{{#if:the president|
           |{{#if:|
               [[Category:Hatnote templates|A{{PAGENAME}}]]
            }}
       }}
     }}

    We split parts at the "|" symbols that are not inside any pair
    {{{...}}}, {{...}}, [[...]], {|...|}.
    """

    # Must consider '[' as normal in expansion of Template:EMedicine2:
    # #ifeq: ped|article|[http://emedicine.medscape.com/article/180-overview|[http://www.emedicine.com/ped/topic180.htm#{{#if: |section~}}
    # as part of:
    # {{#ifeq: ped|article|[http://emedicine.medscape.com/article/180-overview|[http://www.emedicine.com/ped/topic180.htm#{{#if: |section~}}}} ped/180{{#if: |~}}]

    # should handle both tpl arg like:
    #    4|{{{{{subst|}}}CURRENTYEAR}}
    # and tpl parameters like:
    #    ||[[Category:People|{{#if:A|A|{{PAGENAME}}}}]]

    sep = '|'
    parameters = []
    cur = 0
    for s,e in findMatchingBraces(paramsList):
        par = paramsList[cur:s].split(sep)
        if par:
            if parameters:
                # portion before | belongs to previous parameter
                parameters[-1] += par[0]
                if len(par) > 1:
                    # rest are new parameters
                    parameters.extend(par[1:])
            else:
                parameters = par
        elif not parameters:
            parameters = ['']   # create first param
        # add span to last previous parameter
        parameters[-1] += paramsList[s:e]
        cur = e
    # leftover
    par = paramsList[cur:].split(sep)
    if par:
        if parameters:
            # portion before | belongs to previous parameter
            parameters[-1] += par[0]
            if len(par) > 1:
                # rest are new parameters
                parameters.extend(par[1:])
        else:
            parameters = par

    #logging.debug('splitParts %s %s\nparams: %s', sep, paramsList, str(parameters))
    return parameters

def findMatchingBraces(text, ldelim=0):
    """
    :param ldelim: number of braces to match. 0 means match [[]], {{}} and {{{}}}.
    """
    # Parsing is done with respect to pairs of double braces {{..}} delimiting
    # a template, and pairs of triple braces {{{..}}} delimiting a tplarg.
    # If double opening braces are followed by triple closing braces or
    # conversely, this is taken as delimiting a template, with one left-over
    # brace outside it, taken as plain text. For any pattern of braces this
    # defines a set of templates and tplargs such that any two are either
    # separate or nested (not overlapping).

    # Unmatched double rectangular closing brackets can be in a template or
    # tplarg, but unmatched double rectangular opening brackets cannot.
    # Unmatched double or triple closing braces inside a pair of
    # double rectangular brackets are treated as plain text.
    # Other formulation: in ambiguity between template or tplarg on one hand,
    # and a link on the other hand, the structure with the rightmost opening
    # takes precedence, even if this is the opening of a link without any
    # closing, so not producing an actual link.

    # In the case of more than three opening braces the last three are assumed
    # to belong to a tplarg, unless there is no matching triple of closing
    # braces, in which case the last two opening braces are are assumed to
    # belong to a template.

    # We must skip individual { like in:
    #   {{#ifeq: {{padleft:|1|}} | { | | &nbsp;}}
    # We must resolve ambiguities like this:
    #   {{{{ }}}} -> { {{{ }}} }
    #   {{{{{ }}}}} -> {{ {{{ }}} }}
    #   {{#if:{{{{{#if:{{{nominee|}}}|nominee|candidate}}|}}}|...}}

    # Handle:
    #   {{{{{|safesubst:}}}#Invoke:String|replace|{{{1|{{{{{|safesubst:}}}PAGENAME}}}}}|%s+%([^%(]-%)$||plain=false}}
    # as well as expressions with stray }:
    #   {{{link|{{ucfirst:{{{1}}}}}} interchange}}}

    if ldelim:                                   # 2-3
        reOpen = re.compile('[{]{%d,}' % ldelim) # at least ldelim
        reNext = re.compile('[{]{2,}|}{2,}') # at least 2
    else:
        reOpen = re.compile('{{2,}|\[{2,}')
        reNext = re.compile('{{2,}|}{2,}|\[{2,}|]{2,}') # at least 2

    cur = 0
    while True:
        m1 = reOpen.search(text, cur)
        if not m1:
            return
        lmatch = m1.end()-m1.start()
        if m1.group()[0] == '{':
            stack = [lmatch]    # stack of opening braces lengths
        else:
            stack = [-lmatch]   # negative means [
        end = m1.end()
        while True:
            m2 = reNext.search(text, end)
            if not m2:
                return      # unbalanced
            end = m2.end()
            brac = m2.group()[0]
            lmatch = m2.end()-m2.start()

            if brac == '{':
                stack.append(lmatch)
            elif brac == '}':
                while stack:
                    openCount = stack.pop() # opening span
                    if openCount == 0:      # illegal unmatched [[
                        continue
                    if lmatch >= openCount:
                        lmatch -= openCount
                        if lmatch <= 1: # either close or stray }
                            break
                    else:
                        # put back unmatched
                        stack.append(openCount - lmatch)
                        break
                if not stack:
                    yield m1.start(), end-lmatch
                    cur = end
                    break
                elif len(stack) == 1 and 0 < stack[0] < ldelim:
                    # ambiguous {{{{{ }}} }}
                    yield m1.start() + stack[0], end
                    cur = end
                    break
            elif brac == '[': # [[
                stack.append(-lmatch)
            else:               # ]]
                while stack and stack[-1] < 0: # matching [[
                    openCount = -stack.pop()
                    if lmatch >= openCount:
                        lmatch -= openCount
                        if lmatch <= 1: # either close or stray ]
                            break
                    else:
                        # put back unmatched (negative)
                        stack.append(lmatch - openCount)
                        break
                if not stack:
                    yield m1.start(), end-lmatch
                    cur = end
                    break
                # unmatched ]] are discarded
                cur = end

def findBalanced(text, openDelim, closeDelim):
    """
    Assuming that text contains a properly balanced expression using
    :param openDelim: as opening delimiters and
    :param closeDelim: as closing delimiters.
    :return: an iterator producing pairs (start, end) of start and end
    positions in text containing a balanced expression.
    """
    openPat = '|'.join([re.escape(x) for x in openDelim])
    # patter for delimiters expected after each opening delimiter
    afterPat = { o:re.compile(openPat+'|'+c, re.DOTALL) for o,c in izip(openDelim, closeDelim)}
    stack = []
    start = 0
    cur = 0
    end = len(text)
    startSet = False
    startPat = re.compile(openPat)
    nextPat = startPat
    while True:
        next = nextPat.search(text, cur)
        if not next:
            return
        if not startSet:
            start = next.start()
            startSet = True
        delim = next.group(0)
        if delim in openDelim:
            stack.append(delim)
            nextPat = afterPat[delim]
        else:
            opening = stack.pop()
            # assert opening == openDelim[closeDelim.index(next.group(0))]
            if stack:
                nextPat = afterPat[stack[-1]]
            else:
                yield start, next.end()
                nextPat = startPat
                start = next.end()
                startSet = False
        cur = next.end()

# ----------------------------------------------------------------------
# Modules

# Only minimal support
# FIXME: import Lua modules.

modules = {
    'convert' : {
        'convert': lambda x, u, *rest: x+' '+u, # no conversion
    }
}

# ----------------------------------------------------------------------
# variables

class MagicWords(object):
    """
    One copy in each Extractor.

    @see https://doc.wikimedia.org/mediawiki-core/master/php/MagicWord_8php_source.html
    """
    names = [
        '!',
        'currentmonth',
        'currentmonth1',
        'currentmonthname',
        'currentmonthnamegen',
        'currentmonthabbrev',
        'currentday',
        'currentday2',
        'currentdayname',
        'currentyear',
        'currenttime',
        'currenthour',
        'localmonth',
        'localmonth1',
        'localmonthname',
        'localmonthnamegen',
        'localmonthabbrev',
        'localday',
        'localday2',
        'localdayname',
        'localyear',
        'localtime',
        'localhour',
        'numberofarticles',
        'numberoffiles',
        'numberofedits',
        'articlepath',
        'pageid',
        'sitename',
        'server',
        'servername',
        'scriptpath',
        'stylepath',
        'pagename',
        'pagenamee',
        'fullpagename',
        'fullpagenamee',
        'namespace',
        'namespacee',
        'namespacenumber',
        'currentweek',
        'currentdow',
        'localweek',
        'localdow',
        'revisionid',
        'revisionday',
        'revisionday2',
        'revisionmonth',
        'revisionmonth1',
        'revisionyear',
        'revisiontimestamp',
        'revisionuser',
        'revisionsize',
        'subpagename',
        'subpagenamee',
        'talkspace',
        'talkspacee',
        'subjectspace',
        'subjectspacee',
        'talkpagename',
        'talkpagenamee',
        'subjectpagename',
        'subjectpagenamee',
        'numberofusers',
        'numberofactiveusers',
        'numberofpages',
        'currentversion',
        'rootpagename',
        'rootpagenamee',
        'basepagename',
        'basepagenamee',
        'currenttimestamp',
        'localtimestamp',
        'directionmark',
        'contentlanguage',
        'numberofadmins',
        'cascadingsources',
    ]

    def __init__(self):
        self.values = {}
        self.values['!'] = '|'

    def __getitem__(self, name):
        return self.values.get(name)

    def __setitem__(self, name, value):
        self.values[name] = value

    switches = [
        '__NOTOC__',
        '__FORCETOC__',
        '__TOC__',
        '__TOC__',
        '__NEWSECTIONLINK__',
        '__NONEWSECTIONLINK__',
        '__NOGALLERY__',
        '__HIDDENCAT__',
        '__NOCONTENTCONVERT__',
        '__NOCC__',
        '__NOTITLECONVERT__',
        '__NOTC__',
        '__START__',
        '__END__',
        '__INDEX__',
        '__NOINDEX__',
        '__STATICREDIRECT__',
        '__DISAMBIG__'
        ]

magicWordsRE = re.compile('|'.join(MagicWords.switches))

# ----------------------------------------------------------------------
# parser functions utilities

def ucfirst(string):
    """:return: a string with just its first character uppercase
    We can't use title() since it coverts all words.
    """
    if string:
        if len(string) > 1:
            return string[0].upper() + string[1:]
        else:
            return string.upper()
    else:
        return ''

def lcfirst(string):
    """:return: a string with its first character lowercase"""
    if string:
        if len(string) > 1:
            return string[0].lower() + string[1:]
        else:
            return string.lower()
    else:
        return ''

def fullyQualifiedTemplateTitle(templateTitle):
    """
    Determine the namespace of the page being included through the template
    mechanism
    """
    global templatePrefix
    if templateTitle.startswith(':'):
        # Leading colon by itself implies main namespace, so strip this colon
        return ucfirst(templateTitle[1:])
    else:
        m = re.match('([^:]*)(:.*)', templateTitle)
        if m:
            # colon found but not in the first position - check if it
            # designates a known namespace
            prefix = normalizeNamespace(m.group(1))
            if prefix in knownNamespaces:
                return prefix + ucfirst(m.group(2))
    # The title of the page being included is NOT in the main namespace and
    # lacks any other explicit designation of the namespace - therefore, it
    # is resolved to the Template namespace (that's the default for the
    # template inclusion mechanism).

    # This is a defense against pages whose title only contains UTF-8 chars
    # that are reduced to an empty string. Right now I can think of one such
    # case - <C2><A0> which represents the non-breaking space.
    # In this particular case, this page is a redirect to [[Non-nreaking
    # space]], but having in the system a redirect page with an empty title
    # causes numerous problems, so we'll live happier without it.
    if templateTitle:
        return templatePrefix + ucfirst(templateTitle)
    else:
        return ''  # caller may log as error

def normalizeNamespace(ns):
    return ucfirst(ns)

# ----------------------------------------------------------------------
# Parser functions
# see http://www.mediawiki.org/wiki/Help:Extension:ParserFunctions
# https://github.com/Wikia/app/blob/dev/extensions/ParserFunctions/ParserFunctions_body.php

class Infix:
    """Infix operators.
    The calling sequence for the infix is:
      x |op| y
    """
    def __init__(self, function):
        self.function = function
    def __ror__(self, other):
        return Infix(lambda x, self=self, other=other: self.function(other, x))
    def __or__(self, other):
        return self.function(other)
    def __rlshift__(self, other):
        return Infix(lambda x, self=self, other=other: self.function(other, x))
    def __rshift__(self, other):
        return self.function(other)
    def __call__(self, value1, value2):
        return self.function(value1, value2)

ROUND = Infix(lambda x,y: round(x, y))

def sharp_expr(expr):
    try:
        expr = re.sub('=', '==', expr)
        expr = re.sub('mod', '%', expr)
        expr = re.sub('\bdiv\b', '/', expr)
        expr = re.sub('\bround\b', '|ROUND|', expr)
        return unicode(eval(expr))
    except:
        return '<span class="error"></span>'


def sharp_if(testValue, valueIfTrue, valueIfFalse=None, *args):
    # In theory, we should evaluate the first argument here,
    # but it was evaluated while evaluating part[0] in expandTemplate().
    if testValue.strip():
        # The {{#if:}} function is an if-then-else construct.
        # The applied condition is: "The condition string is non-empty".
        valueIfTrue = valueIfTrue.strip()
        if valueIfTrue:
            return valueIfTrue
    elif valueIfFalse:
        return valueIfFalse.strip()
    return ""

def sharp_ifeq(lvalue, rvalue, valueIfTrue, valueIfFalse=None, *args):
    rvalue = rvalue.strip()
    if rvalue:
        # lvalue is always defined
        if lvalue.strip() == rvalue:
            # The {{#ifeq:}} function is an if-then-else construct. The
            # applied condition is "is rvalue equal to lvalue". Note that this
            # does only string comparison while MediaWiki implementation also
            # supports numerical comparissons.

            if valueIfTrue:
                return valueIfTrue.strip()
        else:
            if valueIfFalse:
                return valueIfFalse.strip()
    return ""

def sharp_iferror(test, then='', Else=None, *args):
    if re.match('<(?:strong|span|p|div)\s(?:[^\s>]*\s+)*?class="(?:[^"\s>]*\s+)*?error(?:\s[^">]*)?"', test):
        return then
    elif Else is None:
        return test.strip()
    else:
        return Else.strip()

def sharp_switch(primary, *params):
    # FIXME: we don't support numeric expressions in primary

    # {{#switch: comparison string
    #  | case1 = result1
    #  | case2
    #  | case4 = result2
    #  | 1 | case5 = result3
    #  | #default = result4
    # }}

    primary = primary.strip()
    found = False               # for fall through cases
    default = None
    rvalue = None
    lvalue = ''
    for param in params:
        # handle cases like:
        #  #default = [http://www.perseus.tufts.edu/hopper/text?doc=Perseus...]
        pair = param.split('=', 1)
        lvalue = pair[0].strip()
        rvalue = None
        if len(pair) > 1:
            # got "="
            rvalue = pair[1].strip()
            # check for any of multiple values pipe separated
            if found or primary in [v.strip() for v in lvalue.split('|')]:
                # Found a match, return now
                return rvalue
            elif lvalue == '#default':
                default = rvalue
            rvalue = None   # avoid defaulting to last case
        elif lvalue == primary:
            # If the value matches, set a flag and continue
            found = True
    # Default case
    # Check if the last item had no = sign, thus specifying the default case
    if rvalue is not None:
        return lvalue
    elif default is not None:
        return default
    return ''

# Extension Scribuntu
def sharp_invoke(module, function, frame):
    functions = modules.get(module)
    if functions:
        funct = functions.get(function)
        if funct:
            # find parameters in frame whose title is the one of the original
            # template invocation
            templateTitle = fullyQualifiedTemplateTitle(function)
            if not templateTitle:
                logging.warn("Template with empty title")
            pair = next((x for x in frame if x[0] == templateTitle), None)
            if pair:
                params = pair[1]
                # extract positional args
                params = [params.get(str(i+1)) for i in range(len(params))]
                return funct(*params)
            else:
                return funct()
    return ''

parserFunctions = {

    '#expr': sharp_expr,

    '#if': sharp_if,

    '#ifeq': sharp_ifeq,

    '#iferror': sharp_iferror,

    '#ifexpr': lambda *args: '', # not supported

    '#ifexist': lambda *args: '', # not supported

    '#rel2abs': lambda *args: '', # not supported

    '#switch': sharp_switch,

    '#language': lambda *args: '', # not supported

    '#time': lambda *args: '', # not supported

    '#timel': lambda *args: '', # not supported

    '#titleparts': lambda *args: '', # not supported

    # This function is used in some pages to construct links
    # http://meta.wikimedia.org/wiki/Help:URL
    'urlencode': lambda string, *rest: urllib.quote(string.encode('utf-8')),

    'lc': lambda string, *rest: string.lower() if string else '',

    'lcfirst': lambda string, *rest: lcfirst(string),

    'uc': lambda string, *rest: string.upper() if string else '',

    'ucfirst': lambda string, *rest: ucfirst(string),

    'int': lambda  string, *rest: str(int(string)),

}

def callParserFunction(functionName, args, frame):
    """
    Parser functions have similar syntax as templates, except that
    the first argument is everything after the first colon.
    :return: the result of the invocation, None in case of failure.

    http://meta.wikimedia.org/wiki/Help:ParserFunctions
    """

    try:
       if functionName == '#invoke':
           # special handling of frame
           ret = sharp_invoke(args[0].strip(), args[1].strip(), frame)
           #logging.debug('parserFunction> %s %s', functionName, ret)
           return ret
       if functionName in parserFunctions:
           ret = parserFunctions[functionName](*args)
           #logging.debug('parserFunction> %s %s', functionName, ret)
           return ret
    except:
        return ""             # FIXME: fix errors

    return ""

# ----------------------------------------------------------------------
# Expand using WikiMedia API
# import json

# def expandTemplates(text):
#     """Expand templates invoking MediaWiki API"""
#     text = urlib.urlencodew(text.encode('utf-8'))
#     base = urlbase[:urlbase.rfind('/')]
#     url = base + "/w/api.php?action=expandtemplates&format=json&text=" + text
#     exp = json.loads(urllib.urlopen(url))
#     return exp['expandtemplates']['*']

# ----------------------------------------------------------------------
# Extract Template definition

reNoinclude = re.compile(r'<noinclude>(?:.*?)</noinclude>', re.DOTALL)
reIncludeonly = re.compile(r'<includeonly>|</includeonly>', re.DOTALL)

# These are built before spawning processes, hence thay are shared.
templates = {}
redirects = {}
# cache of parser templates
# FIXME: sharing this with a Manager slows down.
templateCache = {}

def define_template(title, page):
    """
    Adds a template defined in the :param page:.
    @see https://en.wikipedia.org/wiki/Help:Template#Noinclude.2C_includeonly.2C_and_onlyinclude
    """
    global templates
    global redirects

    #title = normalizeTitle(title)

    # check for redirects
    m = re.match('#REDIRECT.*?\[\[([^\]]*)]]', page[0], re.IGNORECASE)
    if m:
        redirects[title] = m.group(1) #normalizeTitle(m.group(1))
        return

    text = unescape(''.join(page))

    # We're storing template text for future inclusion, therefore,
    # remove all <noinclude> text and keep all <includeonly> text
    # (but eliminate <includeonly> tags per se).
    # However, if <onlyinclude> ... </onlyinclude> parts are present,
    # then only keep them and discard the rest of the template body.
    # This is because using <onlyinclude> on a text fragment is
    # equivalent to enclosing it in <includeonly> tags **AND**
    # enclosing all the rest of the template body in <noinclude> tags.

    # remove comments
    text = comment.sub('', text)

    # eliminate <noinclude> fragments
    text = reNoinclude.sub('', text)
    # eliminate unterminated <noinclude> elements
    text = re.sub(r'<noinclude\s*>.*$', '', text, flags=re.DOTALL)
    text = re.sub(r'<noinclude/>', '', text)

    onlyincludeAccumulator = ''
    for m in re.finditer('<onlyinclude>(.*?)</onlyinclude>', text, re.DOTALL):
        onlyincludeAccumulator += m.group(1)
    if onlyincludeAccumulator:
        text = onlyincludeAccumulator
    else:
        text = reIncludeonly.sub('', text)

    if text:
        if title in templates:
            logging.warn('Redefining: %s', title)
        templates[title] = text

# ----------------------------------------------------------------------

def dropNested(text, openDelim, closeDelim):
    """
    A matching function for nested expressions, e.g. namespaces and tables.
    """
    openRE = re.compile(openDelim, re.IGNORECASE)
    closeRE = re.compile(closeDelim, re.IGNORECASE)
    # partition text in separate blocks { } { }
    spans = []                # pairs (s, e) for each partition
    nest = 0                    # nesting level
    start = openRE.search(text, 0)
    if not start:
        return text
    end = closeRE.search(text, start.end())
    next = start
    while end:
        next = openRE.search(text, next.end())
        if not next:            # termination
            while nest:         # close all pending
                nest -=1
                end0 = closeRE.search(text, end.end())
                if end0:
                    end = end0
                else:
                    break
            spans.append((start.start(), end.end()))
            break
        while end.end() < next.start():
            # { } {
            if nest:
                nest -= 1
                # try closing more
                last = end.end()
                end = closeRE.search(text, end.end())
                if not end:     # unbalanced
                    if spans:
                        span = (spans[0][0], last)
                    else:
                        span = (start.start(), last)
                    spans = [span]
                    break
            else:
                spans.append((start.start(), end.end()))
                # advance start, find next close
                start = next
                end = closeRE.search(text, next.end())
                break           # { }
        if next != start:
            # { { }
            nest += 1
    # collect text outside partitions
    return dropSpans(spans, text)

def dropSpans(spans, text):
    """
    Drop from text the blocks identified in :param spans:, possibly nested.
    """
    spans.sort()
    res = ''
    offset = 0
    for s, e in  spans:
        if offset <= s:         # handle nesting
            if offset < s:
                res += text[offset:s]
            offset = e
    res += text[offset:]
    return res

# ----------------------------------------------------------------------
# WikiLinks
# See https://www.mediawiki.org/wiki/Help:Links#Internal_links

# Can be nested [[File:..|..[[..]]..|..]], [[Category:...]], etc.
# Also: [[Help:IPA for Catalan|[andora]]]

def replaceInternalLinks(text):
    """
    Replaces external links of the form:
    [[title |...|label]]trail

    with title concatenated with trail, when present, e.g. 's' for plural.
    """
    # call this after removal of external links, so we need not worry about
    # triple closing ]]].
    cur = 0
    res = ''
    for s,e in findBalanced(text, ['[['], [']]']):
        m = tailRE.match(text, e)
        if m:
            trail = m.group(0)
            end = m.end()
        else:
            trail = ''
            end = e
        inner = text[s+2:e-2]
        # find first |
        pipe = inner.find('|')
        if pipe < 0:
            title = inner
            label = title
        else:
            title = inner[:pipe].rstrip()
            # find last |
            curp = pipe+1
            for s1,e1 in findBalanced(inner, ['[['], [']]']):
                last = inner.rfind('|', curp, s1)
                if last >= 0:
                    pipe = last # advance
                curp = e1
            label = inner[pipe+1:].strip()
        res += text[cur:s] + makeInternalLink(title, label) + trail
        cur = end
    return res + text[cur:]

# the official version is a method in class Parser, similar to this:
# def replaceInternalLinks2(text):
#     global wgExtraInterlanguageLinkPrefixes
 
#     # the % is needed to support urlencoded titles as well
#     tc = Title::legalChars() + '#%'
#     # Match a link having the form [[namespace:link|alternate]]trail
#     e1 = re.compile("([%s]+)(?:\\|(.+?))?]](.*)" % tc, re.S | re.D)
#     # Match cases where there is no "]]", which might still be images
#     e1_img = re.compile("([%s]+)\\|(.*)" % tc, re.S | re.D)
 
#     holders = LinkHolderArray(self)
 
#     # split the entire text string on occurrences of [[
#     iterBrackets = re.compile('[[').finditer(text)

#     m in iterBrackets.next()
#     # get the first element (all text up to first [[)
#     s = text[:m.start()]
#     cur = m.end()

#     line = s
 
#     useLinkPrefixExtension = self.getTargetLanguage().linkPrefixExtension()
#     e2 = None
#     if useLinkPrefixExtension:
#         # Match the end of a line for a word that is not followed by whitespace,
#         # e.g. in the case of "The Arab al[[Razi]]",  "al" will be matched
#         global wgContLang
#         charset = wgContLang.linkPrefixCharset()
#         e2 = re.compile("((?>.*[^charset]|))(.+)", re.S | re.D | re.U)

#     if self.mTitle is None:
#         raise MWException(__METHOD__ + ": \self.mTitle is null\n")

#     nottalk = not self.mTitle.isTalkPage()

#     if useLinkPrefixExtension:
#         m = e2.match(s)
#         if m:
#             first_prefix = m.group(2)
#         else:
#             first_prefix = false
#     else:
#         prefix = ''

#     useSubpages = self.areSubpagesAllowed()
 
#     for m in iterBrackets:
#         line = text[cur:m.start()]
#         cur = m.end()

#         # TODO: Check for excessive memory usage

#         if useLinkPrefixExtension:
#             m = e2.match(e2)
#             if m:
#                 prefix = m.group(2)
#                 s = m.group(1)
#             else:
#                 prefix = ''
#             # first link
#             if first_prefix:
#                 prefix = first_prefix
#                 first_prefix = False
 
#         might_be_img = False
 
#         m = e1.match(line)
#         if m: # page with normal label or alt
#             label = m.group(2)
#             # If we get a ] at the beginning of m.group(3) that means we have a link that is something like:
#             # [[Image:Foo.jpg|[http://example.com desc]]] <- having three ] in a row fucks up,
#             # the real problem is with the e1 regex
#             # See bug 1300.
#             #
#             # Still some problems for cases where the ] is meant to be outside punctuation,
#             # and no image is in sight. See bug 2095.
#             #
#             if label and m.group(3)[0] == ']' and '[' in label:
#                 label += ']' # so that replaceExternalLinks(label) works later
#                 m.group(3) = m.group(3)[1:]
#             # fix up urlencoded title texts
#             if '%' in m.group(1):
#                 # Should anchors '#' also be rejected?
#                 m.group(1) = str_replace(array('<', '>'), array('&lt', '&gt'), rawurldecode(m.group(1)))
#             trail = m.group(3)
#         else:
#             m = e1_img.match(line):
#             if m:
#                 # Invalid, but might be an image with a link in its caption
#                 might_be_img = true
#                 label = m.group(2)
#                 if '%' in m.group(1):
#                     m.group(1) = rawurldecode(m.group(1))
#                 trail = ""
#             else:		# Invalid form; output directly
#                 s += prefix + '[[' + line
#                 continue

#         origLink = m.group(1)

#         # Dont allow internal links to pages containing
#         # PROTO: where PROTO is a valid URL protocol these
#         # should be external links.
#         if (preg_match('/^(?i:' + self.mUrlProtocols + ')/', origLink)) {
#             s += prefix + '[[' + line
#             continue
#         }

#         # Make subpage if necessary
#         if useSubpages:
#             link = self.maybeDoSubpageLink(origLink, label)
#         else:
#             link = origLink

#         noforce = origLink[0] != ':'
#         if not noforce:
#             # Strip off leading ':'
#             link = link[1:]

#         nt = Title::newFromText(self.mStripState.unstripNoWiki(link))
#         if nt is None:
#             s += prefix + '[[' + line
#             continue

#         ns = nt.getNamespace()
#         iw = nt.getInterwiki()

#         if might_be_img {	# if this is actually an invalid link
#             if (ns == NS_FILE and noforce) { # but might be an image
#                 found = False
#                 while True:
#                     # look at the next 'line' to see if we can close it there
#                     next_line = iterBrakets.next()
#                     if not next_line:
#                         break
#                     m = explode(']]', next_line, 3)
#                     if m.lastindex == 3:
#                         # the first ]] closes the inner link, the second the image
#                         found = True
#                         label += "[[%s]]%s" % (m.group(0), m.group(1))
#                         trail = m.group(2)
#                         break
#                     elif m.lastindex == 2:
#                         # if there is exactly one ]] that is fine, we will keep looking
#                         label += "[[{m[0]}]]{m.group(1)}"
#                     else:
#                         # if next_line is invalid too, we need look no further
#                         label += '[[' + next_line
#                         break
#                 if not found:
#                     # we couldnt find the end of this imageLink, so output it raw
#                     # but dont ignore what might be perfectly normal links in the text we ve examined
#                     holders.merge(self.replaceInternalLinks2(label))
#                     s += "{prefix}[[%s|%s" % (link, text)
#                     # note: no trail, because without an end, there *is* no trail
#                     continue
#             } else: # it is not an image, so output it raw
#                 s += "{prefix}[[%s|%s" % (link, text)
#                 # note: no trail, because without an end, there *is* no trail
#                      continue
#         }
 
#         wasblank = (text == '')
#         if wasblank:
#             text = link
#         else:
#             # Bug 4598 madness.  Handle the quotes only if they come from the alternate part
#             # [[Lista d''e paise d''o munno]] . <a href="...">Lista d''e paise d''o munno</a>
#             # [[Criticism of Harry Potter|Criticism of ''Harry Potter'']]
#             #    . <a href="Criticism of Harry Potter">Criticism of <i>Harry Potter</i></a>
#             text = self.doQuotes(text)

#         # Link not escaped by : , create the various objects
#         if noforce and not nt.wasLocalInterwiki():
#             # Interwikis
#             if iw and mOptions.getInterwikiMagic() and nottalk and (
#                     Language::fetchLanguageName(iw, None, 'mw') or
#                     in_array(iw, wgExtraInterlanguageLinkPrefixes)):
#                 # Bug 24502: filter duplicates
#                 if iw not in mLangLinkLanguages:
#                     self.mLangLinkLanguages[iw] = True
#                     self.mOutput.addLanguageLink(nt.getFullText())

#                 s = rstrip(s + prefix)
#                 s += strip(trail, "\n") == '' ? '': prefix + trail
#                 continue

#             if ns == NS_FILE:
#                 if not wfIsBadImage(nt.getDBkey(), self.mTitle):
#                     if wasblank:
#                         # if no parameters were passed, text
#                         # becomes something like "File:Foo.png",
#                         # which we dont want to pass on to the
#                         # image generator
#                         text = ''
#                     else:
#                         # recursively parse links inside the image caption
#                         # actually, this will parse them in any other parameters, too,
#                         # but it might be hard to fix that, and it doesnt matter ATM
#                         text = self.replaceExternalLinks(text)
#                         holders.merge(self.replaceInternalLinks2(text))
#                     # cloak any absolute URLs inside the image markup, so replaceExternalLinks() wont touch them
#                     s += prefix + self.armorLinks(
#                         self.makeImage(nt, text, holders)) + trail
#                 else:
#                     s += prefix + trail
#                 continue
 
#             if ns == NS_CATEGORY:
#                 s = rstrip(s + "\n") # bug 87

#                 if wasblank:
#                     sortkey = self.getDefaultSort()
#                 else:
#                     sortkey = text
#                 sortkey = Sanitizer::decodeCharReferences(sortkey)
#                 sortkey = str_replace("\n", '', sortkey)
#                 sortkey = self.getConverterLanguage().convertCategoryKey(sortkey)
#                 self.mOutput.addCategory(nt.getDBkey(), sortkey)

#                 s += strip(prefix + trail, "\n") == '' ? '' : prefix + trail

#                 continue
#             }
#         }

#         # Self-link checking. For some languages, variants of the title are checked in
#         # LinkHolderArray::doVariants() to allow batching the existence checks necessary
#         # for linking to a different variant.
#         if ns != NS_SPECIAL and nt.equals(self.mTitle) and !nt.hasFragment():
#             s += prefix + Linker::makeSelfLinkObj(nt, text, '', trail)
#                  continue
 
#         # NS_MEDIA is a pseudo-namespace for linking directly to a file
#         # @todo FIXME: Should do batch file existence checks, see comment below
#         if ns == NS_MEDIA:
#             # Give extensions a chance to select the file revision for us
#             options = []
#             descQuery = False
#             Hooks::run('BeforeParserFetchFileAndTitle',
#                        [this, nt, &options, &descQuery])
#             # Fetch and register the file (file title may be different via hooks)
#             file, nt = self.fetchFileAndTitle(nt, options)
#             # Cloak with NOPARSE to avoid replacement in replaceExternalLinks
#             s += prefix + self.armorLinks(
#                 Linker::makeMediaLinkFile(nt, file, text)) + trail
#             continue

#         # Some titles, such as valid special pages or files in foreign repos, should
#         # be shown as bluelinks even though they are not included in the page table
#         #
#         # @todo FIXME: isAlwaysKnown() can be expensive for file links; we should really do
#         # batch file existence checks for NS_FILE and NS_MEDIA
#         if iw == '' and nt.isAlwaysKnown():
#             self.mOutput.addLink(nt)
#             s += self.makeKnownLinkHolder(nt, text, array(), trail, prefix)
#         else:
#             # Links will be added to the output link list after checking
#             s += holders.makeHolder(nt, text, array(), trail, prefix)
#     }
#     return holders

def makeInternalLink(title, label):
    colon = title.find(':')
    if colon > 0 and title[:colon] not in acceptedNamespaces:
        return ''
    if colon == 0:
        # drop also :File:
        colon2 = title.find(':', colon+1)
        if colon2 > 1 and title[colon+1:colon2] not in acceptedNamespaces:
            return ''
    if Extractor.keepLinks:
        return '<a href="%s">%s</a>' % (urllib.quote(title.encode('utf-8')), label)
    else:
        return label

# ----------------------------------------------------------------------
# External links

# from: https://doc.wikimedia.org/mediawiki-core/master/php/DefaultSettings_8php_source.html

wgUrlProtocols = [
     'bitcoin:', 'ftp://', 'ftps://', 'geo:', 'git://', 'gopher://', 'http://',
     'https://', 'irc://', 'ircs://', 'magnet:', 'mailto:', 'mms://', 'news:',
     'nntp://', 'redis://', 'sftp://', 'sip:', 'sips:', 'sms:', 'ssh://',
     'svn://', 'tel:', 'telnet://', 'urn:', 'worldwind://', 'xmpp:', '//'
]

# from: https://doc.wikimedia.org/mediawiki-core/master/php/Parser_8php_source.html

# Constants needed for external link processing
# Everything except bracket, space, or control characters
# \p{Zs} is unicode 'separator, space' category. It covers the space 0x20
# as well as U+3000 is IDEOGRAPHIC SPACE for bug 19052
EXT_LINK_URL_CLASS = r'[^][<>"\x00-\x20\x7F\s]'
ExtLinkBracketedRegex = re.compile('\[(((?i)' + '|'.join(wgUrlProtocols) + ')' + EXT_LINK_URL_CLASS + r'+)\s*([^\]\x00-\x08\x0a-\x1F]*?)\]', re.S | re.U)
EXT_IMAGE_REGEX = re.compile(
    r"""^(http://|https://)([^][<>"\x00-\x20\x7F\s]+)
    /([A-Za-z0-9_.,~%\-+&;#*?!=()@\x80-\xFF]+)\.((?i)gif|png|jpg|jpeg)$""",
    re.X | re.S | re.U)

def replaceExternalLinks(text):
    s = ''
    cur = 0
    for m in ExtLinkBracketedRegex.finditer(text):
        s += text[cur:m.start()]
        cur = m.end()

        url = m.group(1)
        label = m.group(3)
 
        # # The characters '<' and '>' (which were escaped by
        # # removeHTMLtags()) should not be included in
        # # URLs, per RFC 2396.
        # m2 = re.search('&(lt|gt);', url)
        # if m2:
        #     link = url[m2.end():] + ' ' + link
        #     url = url[0:m2.end()]
 
        # If the link text is an image URL, replace it with an <img> tag
        # This happened by accident in the original parser, but some people used it extensively
        m = EXT_IMAGE_REGEX.match(label)
        if m:
            label = makeExternalImage(label)
 
        # Use the encoded URL
        # This means that users can paste URLs directly into the text
        # Funny characters like ö aren't valid in URLs anyway
        # This was changed in August 2004
        s += makeExternalLink(url, label) #+ trail
 
    return s + text[cur:]

# Function applied to wikiLinks
def makeExternalLink(title, anchor):
    colon = title.find(':')
    if colon > 0 and title[:colon] not in acceptedNamespaces:
        return ''
    if colon == 0:
        # drop also :File:
        colon2 = title.find(':', colon+1)
        if colon2 > 1 and title[colon+1:colon2] not in acceptedNamespaces:
            return ''
    if Extractor.keepLinks:
        return '<a href="%s">%s</a>' % (urllib.quote(title.encode('utf-8')), anchor)
    else:
        return anchor

def makeExternalImage(url, alt=''):
    if Extractor.keepLinks:
        return '<img src="%s" alt="%s">' % (url, alt)
    else:
        return alt


# ----------------------------------------------------------------------

# match tail after wikilink
tailRE = re.compile('\w+')

syntaxhighlight = re.compile('&lt;syntaxhighlight .*?&gt;(.*?)&lt;/syntaxhighlight&gt;', re.DOTALL)

expand_templates = True

def clean(extractor, text):
    """
    Transforms wiki markup.
    @see https://www.mediawiki.org/wiki/Help:Formatting
    """

    if (expand_templates):
        # expand templates
        # See: http://www.mediawiki.org/wiki/Help:Templates
        text = extractor.expandTemplates(text)
    else:
        # Drop transclusions (template, parser functions)
        text = dropNested(text, r'{{', r'}}')

    # Drop tables
    text = dropNested(text, r'{\|', r'\|}')

    # replace external links
    text = replaceExternalLinks(text)

    # replace internal links
    text = replaceInternalLinks(text)

    # drop MagicWords behavioral switches
    text = magicWordsRE.sub('', text)

    ################ Process HTML ###############

    # turn into HTML, except for the content of <syntaxhighlight>
    res = ''
    cur = 0
    for m in syntaxhighlight.finditer(text):
        end = m.end()
        res += unescape(text[cur:m.start()]) + m.group(1)
        cur = end
    text = res + unescape(text[cur:])

    # Handle bold/italic/quote
    if extractor.toHTML:
        text = bold_italic.sub(r'<b>\1</b>', text)
        text = bold.sub(r'<b>\1</b>', text)
        text = italic.sub(r'<i>\1</i>', text)
    else:
        text = bold_italic.sub(r'\1', text)
        text = bold.sub(r'\1', text)
        text = italic_quote.sub(r'"\1"', text)
        text = italic.sub(r'"\1"', text)
        text = quote_quote.sub(r'"\1"', text)
    # residuals of unbalanced quotes
    text = text.replace("'''", '').replace("''", '"')

    # Collect spans

    spans = []
    # Drop HTML comments
    for m in comment.finditer(text):
            spans.append((m.start(), m.end()))

    # Drop self-closing tags
    for pattern in selfClosing_tag_patterns:
        for m in pattern.finditer(text):
            spans.append((m.start(), m.end()))

    # Drop ignored tags
    for left, right in ignored_tag_patterns:
        for m in left.finditer(text):
            spans.append((m.start(), m.end()))
        for m in right.finditer(text):
            spans.append((m.start(), m.end()))

    # Bulk remove all spans
    text = dropSpans(spans, text)

    # Drop discarded elements
    for tag in discardElements:
        text = dropNested(text, r'<\s*%s\b[^>/]*>' % tag, r'<\s*/\s*%s>' % tag)

    if not extractor.toHTML:
        # Turn into text what is left (&amp;nbsp;) and <syntaxhighlight>
        text = unescape(text)

    # Expand placeholders
    for pattern, placeholder in placeholder_tag_patterns:
        index = 1
        for match in pattern.finditer(text):
            text = text.replace(match.group(), '%s_%d' % (placeholder, index))
            index += 1

    text = text.replace('<<', u'«').replace('>>', u'»')

    #############################################

    # Cleanup text
    text = text.replace('\t', ' ')
    text = spaces.sub(' ', text)
    text = dots.sub('...', text)
    text = re.sub(u' (,:\.\)\]»)', r'\1', text)
    text = re.sub(u'(\[\(«) ', r'\1', text)
    text = re.sub(r'\n\W+?\n', '\n', text, flags=re.U) # lines with only punctuations
    text = text.replace(',,', ',').replace(',.', '.')

    return text

# skip level 1, it is page name level
section = re.compile(r'(==+)\s*(.*?)\s*\1')

listOpen = { '*': '<ul>', '#': '<ol>', ';': '<dl>', ':': '<dl>' }
listClose = { '*': '</ul>', '#': '</ol>', ';': '</dl>', ':': '</dl>' }
listItem = { '*': '<li>%s</li>', '#': '<li>%s</<li>', ';': '<dt>%s</dt>',
             ':': '<dd>%s</dd>' }

def compact(text):
    """Deal with headers, lists, empty sections, residuals of tables.
    :param toHTML: convert to HTML
    """

    page = []                   # list of paragraph
    headers = {}                # Headers for unfilled sections
    emptySection = False        # empty sections are discarded
    listLevel = ''              # nesting of lists

    for line in text.split('\n'):

        if not line:
            continue
        # Handle section titles
        m = section.match(line)
        if m:
            title = m.group(2)
            lev = len(m.group(1))
            if Extractor.toHTML:
                page.append("<h%d>%s</h%d>" % (lev, title, lev))
            if title and title[-1] not in '!?':
                title += '.'
            headers[lev] = title
            # drop previous headers
            for i in headers.keys():
                if i > lev:
                    del headers[i]
            emptySection = True
            continue
        # Handle page title
        if line.startswith('++'):
            title = line[2:-2]
            if title:
                if title[-1] not in '!?':
                    title += '.'
                page.append(title)
        # handle indents
        elif line[0] == ':':
            #page.append(line.lstrip(':*#;'))
            continue
        # handle lists
        elif line[0] in '*#;:':
            if Extractor.toHTML:
                i = 0
                for c,n in izip_longest(listLevel, line, fillvalue=''):
                    if not n or n not in '*#;:':
                        if c:
                            page.append(listClose[c])
                            listLevel = listLevel[:-1]
                            continue
                        else:
                            break
                    # n != ''
                    if c != n and (not c or (c not in ';:' and n not in ';:')):
                        if c:
                            # close level
                            page.append(listClose[c])
                            listLevel = listLevel[:-1]
                        listLevel += n
                        page.append(listOpen[n])
                    i += 1
                n = line[i-1]   # last list char
                line = line[i:].strip()
                if line: # FIXME: n is '"'
                    page.append(listItem[n] % line)
            else:
                continue
        elif len(listLevel):
            for c in reversed(listLevel):
                page.append(listClose[c])
            listLevel = []

        # Drop residuals of lists
        elif line[0] in '{|' or line[-1] == '}':
            continue
        # Drop irrelevant lines
        elif (line[0] == '(' and line[-1] == ')') or line.strip('.-') == '':
            continue
        elif len(headers):
            if not Extractor.keepSections:
                items = headers.items()
                items.sort()
                for (i, v) in items:
                    page.append(v)
            headers.clear()
            page.append(line)   # first line
            emptySection = False
        elif not emptySection:
            page.append(line)
        # dangerous
        # # Drop preformatted
        # elif line[0] == ' ':
        #     continue

    return page

def handle_unicode(entity):
    numeric_code = int(entity[2:-1])
    if numeric_code >= 0x10000: return ''
    return unichr(numeric_code)

#------------------------------------------------------------------------------
# Output

class NextFile(object):
    """
    Synchronous generation of next available file name.
    """

    filesPerDir = 100

    def __init__(self, path_name):
        self.path_name = path_name
        self.dir_index = -1
        self.file_index = -1

    def next(self):
        self.file_index = (self.file_index + 1) % NextFile.filesPerDir
        if self.file_index == 0:
            self.dir_index += 1
        dirname = self._dirname()
        if not os.path.isdir(dirname):
            os.makedirs(dirname)
        return self._filepath()

    def _dirname(self):
        char1 = self.dir_index % 26
        char2 = self.dir_index / 26 % 26
        return os.path.join(self.path_name, '%c%c' % (ord('A') + char2, ord('A') + char1))

    def _filepath(self):
        return '%s/wiki_%02d' % (self._dirname(), self.file_index)

class OutputSplitter(object):
    """
    File-like object, that splits output to multiple files of a given max size.
    """

    def __init__(self, nextFile, max_file_size=0, compress=True):
        """
        :param nextfile: a NextFile object from which to obtain filenames
            to use.
        :param max_file_size: the maximum size of each file.
        :para compress: whether to write data with bzip compression.
        """
        self.nextFile = nextFile
        self.compress = compress
        self.max_file_size = max_file_size
        self.file = self.open(self.nextFile.next())

    def reserve(self, size):
        if self.file.tell() + size > self.max_file_size:
            self.close()
            self.file = self.open(self.nextFile.next())

    def write(self, data):
        self.reserve(len(data))
        self.file.write(data)

    def close(self):
        self.file.close()

    def open(self, filename):
        if self.compress:
            return bz2.BZ2File(filename + '.bz2', 'w')
        else:
            return open(filename, 'w')

# ----------------------------------------------------------------------
# READER

tagRE = re.compile(r'(.*?)<(/?\w+)[^>]*>(?:([^<]*)(<.*?>)?)?')
#                    1     2               3      4

def load_templates(file, output_file=None):
    """
    Load templates from :param file:.
    :param output_file: file where to save templates and modules.
    """
    global templateNamespace, templatePrefix
    templatePrefix = templateNamespace + ':'
    global moduleNamespace, modulePrefix
    modulePrefix = moduleNamespace + ':'
    articles = 0
    page = []
    inText = False
    if output_file:
        output = codecs.open(output_file, 'wb', 'utf-8')
    for line in file:
        line = line.decode('utf-8')
        if '<' not in line:         # faster than doing re.search()
            if inText:
                page.append(line)
            continue
        m = tagRE.search(line)
        if not m:
            continue
        tag = m.group(2)
        if tag == 'page':
            page = []
        elif tag == 'title':
            title = m.group(3)
        elif tag == 'text':
            inText = True
            line = line[m.start(3):m.end(3)]
            page.append(line)
            if m.lastindex == 4: # open-close
                inText = False
        elif tag == '/text':
            if m.group(1):
                page.append(m.group(1))
            inText = False
        elif inText:
            page.append(line)
        elif tag == '/page':
            if not output_file and not templateNamespace: # do not know it yet
                # we reconstruct it from the first title
                colon = title.find(':')
                if colon > 1:
                    templateNamespace = title[:colon]
                    templatePrefix = title[:colon+1]
            # FIXME: should reconstruct also moduleNamespace
            if title.startswith(templatePrefix):
                define_template(title, page)
            # save templates and modules to file
            if output_file and (title.startswith(templatePrefix) or
                                title.startswith(modulePrefix)):
                output.write('<page>\n')
                output.write('   <title>%s</title>\n' % title)
                output.write('   <ns>10</ns>\n')
                output.write('   <text>')
                for line in page:
                    output.write(line)
                output.write('   </text>\n')
                output.write('</page>\n')
            page = []
            articles += 1
            if articles % 100000 == 0:
                logging.info("Preprocessed %d pages", articles)
    if output_file:
        output.close()
        logging.info("Saved %d templates to '%s'", len(templates), output_file)

def process_dump(input_file, template_file, out_file, file_size, file_compress,
                 process_count):
    """
    :param input_file: name of the wikipedia dump file; '-' to read from stdin
    :param template_file: optional file with template definitions.
    :param out_file: directory where to store extracted data, or '-' for stdout
    :param file_size: max size of each extracted file, or None for no max (one file)
    :param file_compress: whether to compress files with bzip.
    :param process_count: number of extraction processes to spawn.
    """
    global urlbase
    global knownNamespaces
    global templateNamespace, templatePrefix
    global moduleNamespace, modulePrefix
    global expand_templates

    if input_file == '-':
        input = sys.stdin
    else:
        input = fileinput.FileInput(input_file, openhook=fileinput.hook_compressed)

    # collect siteinfo
    for line in input:
        line = line.decode('utf-8')
        m = tagRE.search(line)
        if not m:
            continue
        tag = m.group(2)
        if tag == 'base':
            # discover urlbase from the xml dump file
            # /mediawiki/siteinfo/base
            base = m.group(3)
            urlbase = base[:base.rfind("/")]
        elif tag == 'namespace':
            knownNamespaces.add(m.group(3))
            if re.search('key="10"', line):
                templateNamespace = m.group(3)
                templatePrefix = templateNamespace + ':'
            elif re.search('key="828"', line):
                moduleNamespace = m.group(3)
                modulePrefix = moduleNamespace + ':'
        elif tag == '/siteinfo':
            break

    if expand_templates:
        # preprocess
        template_load_start = default_timer()
        if template_file and os.path.exists(template_file):
            logging.info("Preprocessing '%s' to collect template definitions: this may take some time.", template_file)
            file = fileinput.FileInput(template_file, openhook=fileinput.hook_compressed)
            load_templates(file)
            file.close()
        else:
            if input_file == '-':
                # can't scan then reset stdin; must error w/ suggestion to specify template_file
                raise ValueError("to use templates with stdin dump, must supply explicit template-file")
            logging.info("Preprocessing '%s' to collect template definitions: this may take some time.", input_file)
            load_templates(input, template_file)
            input.close()
            input = fileinput.FileInput(input_file,openhook=fileinput.hook_compressed)
        template_load_elapsed = default_timer() - template_load_start
        logging.info("Loaded %d templates in %.1fs", len(templates), template_load_elapsed)

    if out_file == '-':
        output = sys.stdout
        if file_compress:
            logging.warn("writing to stdout, so no output compression (use external tool)")
    else:
        nextFile = NextFile(out_file)
        output = OutputSplitter(nextFile, file_size, file_compress)

    # process pages
    logging.info("Starting page extraction from %s.", input_file)
    extract_start = default_timer()

    # Parallel Map/Reduce:
    # - pages to be processed are dispatched to workers
    # - a reduce process collects the results, sort them and print them.

    maxsize = 10 * process_count
    # output queue
    output_queue = Queue(maxsize=maxsize)

    # Reduce job that sorts and prints output
    reduce = Process(target=reduce_process, args=(output_queue, output))
    reduce.start()

    # initialize jobs queue
    jobs_queue = Queue(maxsize=maxsize)

    # start worker processes
    logging.info("Using %d extract processes.", process_count)
    workers = []
    for _ in xrange(max(1, process_count)):
        extractor = Process(target=extract_process,
                            args=(jobs_queue, output_queue))
        extractor.daemon = True  # only live while parent process lives
        extractor.start()
        workers.append(extractor)

    # Mapper process

    # we collect individual lines, since str.join() is significantly faster
    # than concatenation
    page = []
    id = None
    last_id = None
    ordinal = 0                 # page count
    inText = False
    redirect = False
    for line in input:
        line = line.decode('utf-8')
        if '<' not in line:         # faster than doing re.search()
            if inText:
                page.append(line)
            continue
        m = tagRE.search(line)
        if not m:
            continue
        tag = m.group(2)
        if tag == 'page':
            page = []
            redirect = False
        elif tag == 'id' and not id:
            id = m.group(3)
        elif tag == 'title':
            title = m.group(3)
        elif tag == 'redirect':
            redirect = True
        elif tag == 'text':
            inText = True
            line = line[m.start(3):m.end(3)]
            page.append(line)
            if m.lastindex == 4: # open-close
                inText = False
        elif tag == '/text':
            if m.group(1):
                page.append(m.group(1))
            inText = False
        elif inText:
            page.append(line)
        elif tag == '/page':
            colon = title.find(':')
            if (colon < 0 or title[:colon] in acceptedNamespaces) and id != last_id and \
                    not redirect and not title.startswith(templateNamespace):
                job = (id, title, page, ordinal)
                jobs_queue.put(job) # goes to any available extract_process
                last_id = id
                ordinal += 1
            id = None
            page = []

    input.close()

    # signal termination
    for w in workers:
        jobs_queue.put(None)
    # wait for workers to terminate
    for w in workers:
        w.join()

    # signal end of work to reduce process
    output_queue.put(None)
    # wait for it to finish
    reduce.join()

    if output != sys.stdout:
        output.close()
    extract_duration = default_timer() - extract_start
    extract_rate = ordinal / extract_duration
    logging.info("Finished %d-process extraction of %d articles in %.1fs (%.1f art/s)",
                 process_count, ordinal, extract_duration, extract_rate)

#----------------------------------------------------------------------
# Multiprocess support

def extract_process(jobs_queue, output_queue):
    """Pull tuples of raw page content, do CPU/regex-heavy fixup, push finished text
    :param job_queue: where to get jobs.
    :param output_queue: where to queue extracted text for output.
    """
    while True:
        job = jobs_queue.get()  # job is (id, title, page, ordinal)
        if job:
            out = StringIO()    # memory buffer
            Extractor(*job[:3]).extract(out) # (id, title, page)
            text = out.getvalue()
            output_queue.put((job[3], text)) # (ordinal, extracted_text)
            out.close()
        else:
            break

def reduce_process(output_queue, output):
    """Pull finished article text, write series of files (or stdout)
    :param output_queue: text to be output.
    :param output: file object where to print.
    """

    interval_start = default_timer()
    period = 100000
    # FIXME: use a heap
    ordering_buffer = {}        # collected pages
    next_ordinal = 0 # sequence number of pages
    while True:
        if next_ordinal in ordering_buffer:
            output.write(ordering_buffer.pop(next_ordinal))
            next_ordinal += 1
            # progress report
            if next_ordinal % period == 0:
                interval_rate = period / (default_timer() - interval_start)
                logging.info("Extracted %d articles (%.1f art/s)",
                             next_ordinal, interval_rate)
                interval_start = default_timer()
        else:
            # mapper puts None to signal finish
            pair = output_queue.get()
            if not pair:
                break
            ordinal, text = pair
            ordering_buffer[ordinal] = text

# ----------------------------------------------------------------------

# Minimum size of output files
minFileSize = 200 * 1024

def main():
    global urlbase, acceptedNamespaces
    global expand_templates, templateCache

    parser = argparse.ArgumentParser(prog=os.path.basename(sys.argv[0]),
        formatter_class=argparse.RawDescriptionHelpFormatter,
                                     description=__doc__)
    parser.add_argument("input",
                        help="XML wiki dump file")
    groupO = parser.add_argument_group('Output')
    groupO.add_argument("-o", "--output", default="text",
                        help="directory for extracted files (or '-' for dumping to stdin)")
    groupO.add_argument("-b", "--bytes", default="1M",
                        help="maximum bytes per output file (default %(default)s)",
                        metavar="n[KMG]")
    groupO.add_argument("-c", "--compress", action="store_true",
                        help="compress output files using bzip")

    groupP = parser.add_argument_group('Processing')
    groupP.add_argument("--html", action="store_true",
                        help="produce HTML output, subsumes --links and --sections")
    groupP.add_argument("-l", "--links", action="store_true",
                        help="preserve links")
    groupP.add_argument("-ns", "--namespaces", default="", metavar="ns1,ns2",
                        help="accepted namespaces")
    groupP.add_argument("-s", "--sections", action="store_true",
                        help="preserve sections")
    groupP.add_argument("--templates",
                        help="use or create file containing templates")
    groupP.add_argument("--no-templates", action="store_false",
                        help="Do not expand templates")
    default_process_count = cpu_count() - 1
    parser.add_argument("--processes", type=int, default=default_process_count,
                        help="Number of extract processes (default %(default)s)")

    groupS = parser.add_argument_group('Special')
    groupS.add_argument("-q", "--quiet", action="store_true",
                        help="suppress reporting progress info")
    groupS.add_argument("--debug", action="store_true",
                        help="print debug info")
    groupS.add_argument("-a", "--article", action="store_true",
                        help="analyze a file containing a single article (debug option)")
    groupS.add_argument("-v", "--version", action="version",
                        version='%(prog)s ' + version,
                        help="print program version")

    args = parser.parse_args()

    Extractor.keepLinks = args.links
    Extractor.keepSections = args.sections
    Extractor.toHTML = args.html
    if args.html:
        Extractor.keepLinks = True
        Extractor.keepSections = True

    expand_templates = args.no_templates

    try:
        power = 'kmg'.find(args.bytes[-1].lower()) + 1
        file_size = int(args.bytes[:-1]) * 1024 ** power
        if file_size < minFileSize:
            raise ValueError()
    except ValueError:
        logging.error('Insufficient or invalid size: %s', args.bytes)
        return

    if args.namespaces:
        acceptedNamespaces = set(args.namespaces.split(','))

    FORMAT = '%(levelname)s: %(message)s'
    logging.basicConfig(format=FORMAT)

    logger = logging.getLogger()
    if not args.quiet:
        logger.setLevel(logging.INFO)
    if args.debug:
        logger.setLevel(logging.DEBUG)

    input_file = args.input

    if not Extractor.keepLinks:
        ignoreTag('a')

    # sharing cache of parser templates is too slow:
    #manager = Manager()
    #templateCache = manager.dict()

    if args.article:
        if args.templates:
            if os.path.exists(args.templates):
                with open(args.templates) as file:
                    load_templates(file)

        with open(input_file) as file:
            page = file.read().decode('utf-8')
            m = re.search(r'<id>(.*)</id>', page)
            id = m.group(1) if m else 0
            m = re.search(r'<title>(.*)</title>', page)
            if m:
                title = m.group(1)
            else:
                logging.error('Missing title element')
                return
            Extractor(id, title, [page]).extract(sys.stdout)
        return

    output_path = args.output
    if output_path != '-' and not os.path.isdir(output_path):
        try:
            os.makedirs(output_path)
        except:
            logging.error('Could not create: %s', output_path)
            return

    process_dump(input_file, args.templates, output_path, file_size,
                 args.compress, args.processes)


if __name__ == '__main__':
    main()
