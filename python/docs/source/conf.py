# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

import os
import sys
import vowpalwabbit
from pathlib import Path

# -- Project information -----------------------------------------------------

project = "VowpalWabbit"
copyright = "2021, John langford et al"
author = "John langford et al"

# Read version automatically from vowpalwabbit.__version__ or use env var
override_package_version = os.getenv("VW_SPHINX_VERSION_OVERRIDE")
if override_package_version != None:
    version = override_package_version
else:
    version = vowpalwabbit.__version__

release = version

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.doctest",
    "sphinx.ext.intersphinx",
    "sphinx.ext.coverage",
    "numpydoc",
    "sphinx.ext.githubpages",
    "sphinx_thebe",
    "myst_nb",
    "sphinx_reredirects",
]

numpydoc_show_class_members = False
autodoc_typehints_format = "short"

templates_path = ["_templates"]

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
source_suffix = [".rst", ".md"]

# The master toctree document.
master_doc = "index"

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = None

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "pydata_sphinx_theme"

html_theme_options = {"use_edit_page_button": True}

jupyter_execute_notebooks = "cache"

# This tutorial uses unrar so we can't execute it in the doc generation.
execution_excludepatterns = ["DFtoVW_tutorial.ipynb"]

thebe_config = {
    "repository_url": "https://github.com/VowpalWabbit/vowpal_wabbit",
    "repository_branch": "master",
    "selector": ".cell",
}
show_navbar_depth = 2

myst_heading_anchors = 2

html_context = {
    "github_user": "VowpalWabbit",
    "github_repo": "vowpal_wabbit",
    "github_version": "master",
    "doc_path": "python/docs/source",
}

# These suffixes are needed otherwise it chops off DFtoVW etc thinking it is a suffix
redirects = {
    "vowpalwabbit.DFtoVW.rst": "reference/vowpalwabbit.DFtoVW.html",
    "vowpalwabbit.pyvw.rst": "reference/vowpalwabbit.pyvw.html",
    "vowpalwabbit.sklearn.rst": "reference/vowpalwabbit.sklearn.html",
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]

html_css_files = ["custom.css", "nav.css"]

html_sidebars = {"**": ["search-field.html", "nav-toc-override.html"]}

# -- Options for intersphinx extension ---------------------------------------

# Example configuration for intersphinx: refer to the Python standard library.
intersphinx_mapping = {"https://docs.python.org/": None}

html_favicon = "favicon.png"

binder_url_config = {
    "branch": "master",
    "repo_url": "https://github.com/VowpalWabbit/vowpal_wabbit",
    "path_to_docs": "python/docs/source",
}


def add_binder_url_for_page(
    app,
    pagename: str,
    templatename: str,
    context,
    doctree,
):

    # First decide if we'll insert any links
    path = app.env.doc2path(pagename)
    extension = Path(path).suffix
    binder_url_config = app.config["binder_url_config"]

    repo_url = binder_url_config["repo_url"]

    # Parse the repo parts from the URL
    org, repo = _split_repo_url(repo_url)
    if org is None and repo is None:
        # Skip the rest because the repo_url isn't right
        return

    branch = binder_url_config["branch"]

    # Check if we have a non-ipynb file, but an ipynb of same name exists
    # If so, we'll use the ipynb extension instead of the text extension
    if extension != ".ipynb" and Path(path).with_suffix(".ipynb").exists():
        extension = ".ipynb"
    elif extension != ".ipynb":
        return

    # If this is an excluded notebook, then skip it.
    if Path(path).name in app.config["execution_excludepatterns"]:
        return

    # Construct a path to the file relative to the repository root
    book_relpath = binder_url_config["path_to_docs"]
    if book_relpath != "":
        book_relpath += "/"
    path_rel_repo = f"{book_relpath}{pagename}{extension}"

    url = (
        f"https://mybinder.org/v2/gh/{org}/{repo}/{branch}?" f"filepath={path_rel_repo}"
    )
    context["binder_url"] = url


def _split_repo_url(url):
    """Split a repository URL into an org / repo combination."""
    if "github.com/" in url:
        end = url.split("github.com/")[-1]
        org, repo = end.split("/")[:2]
    else:
        raise ValueError(
            f"Currently Binder/JupyterHub repositories must be on GitHub, got {url}"
        )
    return org, repo


# This helps to document __init__
def skip(app, what, name, obj, would_skip, options):
    if name == "__init__":
        return False
    return would_skip


def setup(app):
    app.add_config_value("binder_url_config", {}, "html")
    app.connect("autodoc-skip-member", skip)
    app.connect("html-page-context", add_binder_url_for_page)
