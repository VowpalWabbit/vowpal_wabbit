---
title: "VowpalWabbit {{version}} Release Notes"
layout: blog
tags: Release&nbsp;notes
description: <!--EDIT_ME-->
author: <!--EDIT_ME-->
avatar_link: <!--EDIT_ME-->
---

<!--Instructions on using: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Release-Process#generating-changelist -->

<div class="blog_highlight" markdown="1">
- [GitHub release](https://github.com/VowpalWabbit/vowpal_wabbit/releases/tag/{{version}})
- [PyPi](https://pypi.org/project/vowpalwabbit/)
</div>

<!--EDIT_ME: Introduction-->

## Breaking changes

<!--EDIT_ME: add ### section for each breaking change-->

{{#feat_type_commits}}
{{#breaking}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/breaking}}
{{/feat_type_commits}}

## Highlights
<!--EDIT_ME: add ### section and break down most interesting changes-->

## Thank you

A huge thank you and welcome to all of the new contributors since the last release:

<!--EDIT_ME: determine who is new and fix their github username-->
{{#authors}}
- [@{{name}}]((https://github.com/{{name}}))
{{/authors}}

And of course thank you to existing contributors:

<!--EDIT_ME: determine who is new and fix their github username-->
{{#authors}}
- [@{{name}}]((https://github.com/{{name}}))
{{/authors}}

<div>
  <i class="fa fa-caret-right"></i>
  <button class="changelist_button">
    Click here to expand and see the full changelist.
  </button>
</div>
<div class="changelist hidden" markdown="1">

## Features
{{#feat_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/feat_type_commits}}

## Fixes
{{#fix_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/fix_type_commits}}

## Other changes
{{#docs_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/docs_type_commits}}
{{#style_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/style_type_commits}}
{{#refactor_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/refactor_type_commits}}
{{#perf_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/perf_type_commits}}
{{#test_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/test_type_commits}}
{{#build_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/build_type_commits}}
{{#ci_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/ci_type_commits}}
{{#chore_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/chore_type_commits}}
{{#revert_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/revert_type_commits}}
{{#unknown_type_commits}}
-[{{subject}}]({{github_pr_url}}{{pr_num}})
{{/unknown_type_commits}}

</div>
