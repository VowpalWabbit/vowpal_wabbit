# Contributing

Vowpal Wabbit welcomes contributions from the community. Consider this document
as a guideline rather than strict rules. This are just a set of suggestions that
we find useful to increase productivitity for everyone involved.

## Process

1. Make a proposal.
1. Implement the proposal and its tests.
1. Squash commits and write a good commit message.
1. Start a pull request & address comments.
1. Merge.

### Proposal

For things like fixing typos and small bug fixes, you can skip this step.

If your change is more than a simple fix, please don't just create a big
pull request. Instead, start by opening an issue describing the problem you
want to solve and how you plan to approach the problem. This will let us
have a brief discussion about the problem and, hopefully, identify some
potential pitfalls before too much time is spent on your part.

### Implementation

* Fork the repository on GitHub.
* Start on a new topic branch off of master.
* Instructions for getting Vowpal Wabbit building and running the tests are in the
  [Wiki](https://github.com/VowpalWabbit/vowpal_wabbit/wiki).
* Aim for each pull request to have one goal. If the commit starts to get
  too large, consider splitting it into multiple, independent pull requests.
    * Some changes are more naturally authored in the same pull request. This is
      fine, though we find this to be a rare occurrence. These sort of changes
      are harder to review and interate on with GitHub's tooling.
* Aim to add tests that exercise the new behaviour and make sure that all the 
  tests continue to pass.

### Suggested commit structure

Feel free to use the pull request description as the final commit message that
best describes the work done. This description will be copied by one of the
maintainers when merging the PR.

In your commit message, explain the reasoning behind the commit. The code
changes answer the question "What changed?", but the commit message answers
the question "Why did it need to change?".

Please follow the established Git
[convention for commit messages](https://www.git-scm.com/book/en/v2/Distributed-Git-Contributing-to-a-Project#Commit-Guidelines).
The first line is a summary in the imperative, about 50 characters or less,
and should *not* end with a period. An optional, longer description must be
preceded by an empty line and should be wrapped at around 72 characters.
This helps with various outputs from Git or other tools.

Don't forget to run `git diff --check` to catch those annoying whitespace
changes.

You can update messages of local commits you haven't pushed yet using `git
commit --amend` or `git rebase --interactive` with *reword* command.

### Pull request

Start a GitHub pull request to merge your topic branch into the
[main repository's master branch](https://github.com/VowpalWabbit/vowpal_wabbit/tree/master).

When you submit a pull request, a suite of builds and tests will be run
automatically, and the results will show up in the "Checks" section of the
PR. If any of these fail, you'll need to figure out why and make the
appropriate fixes. If you think the failures are due to infrastructure
issues, please mention this in a comment, and one of the maintainers will
help.

The project maintainers will review your changes. We aim to review all
changes within three business days.

Address any review comments by adding commits that address the comments.
Don't worry about having fixup/tiny commits at this stage. They'll get
squashed together when the change is finally merged. Push these new commits
to your topic branch, and we'll review the edits.

### Merge

Once the comments in the pull request have been addressed, a project
maintainer will merge your changes. Thank you for helping improve Vowpal Wabbit!

By default we'll perform a squash merge unless requested otherwise in the
PR.

# Reporting Security Issues

Security issues and bugs should be reported privately, via email, to *insert
email* 

# Code of Conduct

Although VW is not a Microsoft project, we follow the 
[Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the
[Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)

