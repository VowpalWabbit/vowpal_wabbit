Document Maintenance
====================

Document Software Setup
-----------------------

Vowpal Wabbit python Wrapper documentation is built using Sphinx.
See http://www.sphinx-doc.org/en/master/index.html


You can create, build, and preview the documentation on your local machine.

You can commit your updates to the vowpal wabbit repository on GitHub, which triggers an automatic rebuild.


Clone Vowpal Wabbit
-------------------

To get started, create or identify a working directory on your local machine.

Open that directory and execute the following command in a terminal session::

    git clone https://github.com/VowpalWabbit/vowpal_wabbit.git

That will create an /vowpal_wabbit directory in your working directory.

Now you can build the HTML documents locally::

    cd vowpal_wabbit/python/docs/
    make html

Assuming that your Sphinx installation was successful, Sphinx should build a local instance of the
documentation .html files::

    open build/html/index.html

In case this command did not work, for example on Ubuntu 18.04 you may get a message like “Couldn’t
get a file descriptor referring to the console”, try: ::

    see build/html/index.html

You now have a local build of the documents.

Improve Vowpal Wabbit Wrapper Documents
---------------------------------------

Ensure that you have the latest Documents files::

    git pull
    git status

Use your favorite text editor to create and modify .rst files to make your documentation
improvements.



Converting from Markdown
------------------------

If you want to convert a ``.md`` file to a ``.rst`` file, this `tool <https://github.com/chrissimpkins/md2rst>`_
does it pretty well. You'd still have to clean up and check for errors as this contains a lot of
bugs. But this is definitely better than converting everything by yourself.

This will be helpful in converting GitHub wiki's (Markdown Files) to reStructuredtext files for
Sphinx/ReadTheDocs hosting.

