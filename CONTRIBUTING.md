# Contributing

Vowpal Wabbit thrives on its community. This document contains some guidelines
to make contributing easier.

## Process

1. [Reach out if you want feedback](#reaching-out)
1. [Implement the change and its tests](#implementation)
1. [Start a pull request & address comments](#pull-request)
1. [Merge](#merge)

### Reaching out

Open an issue if you want feedback on your idea before you code. If it's
something non trivial we suggest reaching out to make sure it aligns with the
project goals (performance implications, design requirements, interfaces, etc.).
This will let us have a brief discussion about the problem and, hopefully,
identify some potential pitfalls before too much time is spent on your part.

If you're just fixing typos or small bugs there's no need to reach out
beforehand.

### Implementation

* Fork the repository on GitHub
* Start on a new topic branch off of master
* Instructions for getting Vowpal Wabbit building and running the tests are in the
  [Wiki](https://github.com/VowpalWabbit/vowpal_wabbit/wiki)
* Aim for each pull request to have one goal. If the PR starts to get too large,
  consider splitting it into multiple, independent pull requests
    * Some changes are more naturally authored in the same pull request. This is
      fine, though we find this to be a rare occurrence. These sort of changes
      are harder to review
* Aim to add tests that exercise the new behaviour and make sure that all the
  tests continue to pass

### Pull request

Start a GitHub pull request to merge your topic branch into the [main repository's master branch](https://github.com/VowpalWabbit/vowpal_wabbit/tree/master).

When you submit a pull request, a suite of builds and tests will be run
automatically, and the results will show up in the "Checks" section of the PR.
Generally, we'll wait for these to all pass before we review your PR. If you
need help resolving build or test issues feel free to reach out in the comments
of your PR.

We suggest ticking "Allow edits and access to secrets by maintainers" so that we
can contribute to your PR if need be.

### Merge

Once the comments in the pull request have been addressed, we will merge your
changes. Thank you for helping improve Vowpal Wabbit!

By default we'll perform a squash merge, so don't worry if the commit history is
messy.

# Reporting Security Issues

Security issues and bugs should be reported privately, via email, to `vowpalwabbit-security@hunch.net`
# Contact us

Ways to get in contact can be found [here](https://github.com/VowpalWabbit/vowpal_wabbit/issues/new/choose).

# Code of Conduct

Although VW is not a Microsoft project, we follow the 
[Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the
[Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
