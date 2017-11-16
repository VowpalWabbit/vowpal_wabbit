@echo off
c:\Python34\Scripts\pip.exe uninstall vowpalwabbit
c:\Python34\python.exe setup.py clean
c:\Python34\python.exe setup.py sdist
c:\Python34\Scripts\pip.exe install -v dist\vowpalwabbit-8.3.0.tar.gz