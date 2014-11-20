Instructions for installation of FunctionalTests for MonaServer.

To create symbolic links on Windows you can use "NTFS Link Ext" (http://sourceforge.net/projects/ntfslinkext/).

1. First of all, if directories "www" and "data" are not presents in the execution directory of Mona, create them!
2. Then create a symbolic link of "www/FunctionalTests" in the directory "www" of MonaServer.
3. And create a symbolic link of "data/FunctionalTests" in the directory "data" of MonaServer.
4. Restart MonaServer.
5. You can now run the tests from http://localhost/FunctionalTests/.

That's all!