This is contributed win32 configuration but I cannot test it since I
don't use Windows.  Please send me patches to fix things that are
wrong here.

raptor.dsp assumes that expat and curl has been installed or compiled
in sibling top level directories.  The versions used here look for:

  ..\..\curl-7.12.1\lib\curllib.dsp
  ..\..\expat-1.95.8\lib\expat.dsp


It should be relatively easy to change it to use libxml2 with some
edits to ..\win32_config.h - remove the expat references and add:
  #define RAPTOR_XML_LIBXML 1
  #define HAVE_LIBXML_PARSER_H 1
  #define HAVE_LIBXML_HASH_H 1
  #define RAPTOR_LIBXML_ENTITY_ETYPE 1
  #define RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET 1
  #define RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED 1
(these will vary a lot depending on the libxml2 version).
and the corresponding library changes to raptor.dsp

Dave
