/* package name */
#define PACKAGE

/* package version */
#define VERSION

#undef RAPTOR_VERSION_MAJOR
#undef RAPTOR_VERSION_MINOR
#undef RAPTOR_VERSION_RELEASE

/* XML parsers */
#undef RAPTOR_XML_EXPAT
#undef RAPTOR_XML_LIBXML

/* expat compiled with namespaces? */
#undef HAVE_XML_SetNamespaceDeclHandler

/* need 'extern int optind' declaration? */
#undef NEED_OPTIND_DECLARATION

/* does expat crash when it sees an initial UTF8 BOM? */
#undef EXPAT_UTF8_BOM_CRASH

/* does libxml struct xmlEntity have a field name_length */
#undef RAPTOR_LIBXML_ENTITY_NAME_LENGTH

/* does libxml struct xmlEntity have a field etype */
#undef RAPTOR_LIBXML_ENTITY_ETYPE

/* is this being built inside redland? */
#undef RAPTOR_IN_REDLAND
