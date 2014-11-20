.. installation and usage guide of Mona rst documentation

RST : Documentation WYSIWYG format
##########################################

ReStructuredText is an easy-to-read, what-you-see-is-what-you-get plaintext markup syntax and parser system used for for example in **Python**'s documentation.
It is really powerful and can produce documentation in **many formats**. One of the most interessant thing is that it can be "versioned", this is an easy way to check for modifications.

Please take a look to the website of Sphinx_ for more informations.

RST Installation for Windows
******************************************

Sphinx_ Installation
-------------------------------------

RST files can produce several formats (pdf, html...) with the help of Sphinx_ tool.

* First you nead to install Python_ and `Easy Install`_, the most easy way for this is to download ActivePython_, a usefall installer that contains `Easy Install`_ and several Python_ librairies.
* Then you need to install Sphinx_, nothing easier, just run the following command :

.. code-block:: sh

	$ easy_install -U Sphinx

Theme installation
-------------------------------------

Actually we use the `Sphinx Bootstrap theme`_ for html generation. To install it just run the command bellow :

.. code-block:: sh

  $ easy_install -U sphinx_bootstrap_theme
 
Usage of Sphinx_
-------------------------------------

Just run the command bellow :

.. code-block:: sh

  $ make html
  
.. note:: Result is available in directory *./_build/html*.

If you use Notepad++ (plugin NppExec)
----------------------------------------------

* Install plugin NppExec (*Plugins>Plugin Manager>Show Plugin Manager*),
* Add command to NppExec :
	* Menu *Plugins>NppExec>Execute...*,
	* Write the script bellow :

	.. code-block:: sh

		cd $(CURRENT_DIRECTORY)
		$(CURRENT_DIRECTORY)/make.bat html
	
	* And save the script (type 'rst2html' for example).

To run the script just type **F6** then **Enter**.

If you use Komodo Edit
----------------------------------------------

* Tools>Run Command...
* In "Run" type "make html"
* In "Start in" choose the path of "doc" (this directory)
* Then just run it.

.. _Sphinx : http://sphinx-doc.org/index.html
.. _`Easy Install` : http://pythonhosted.org/setuptools/easy_install.html
.. _Python : http://www.python.org/
.. _ActivePython : http://www.activestate.com/activepython
.. _`Sphinx Bootstrap theme` : https://github.com/ryan-roemer/sphinx-bootstrap-theme
