/* package name */
#define PACKAGE

/* package version */
#define VERSION

#undef RAPTOR_VERSION_MAJOR
#undef RAPTOR_VERSION_MINOR
#undef RAPTOR_VERSION_RELEASE

/* XML parsers */
#undef NEED_EXPAT
#undef NEED_LIBXML

/* expat compiled with namespaces? */
#undef HAVE_XML_SetNamespaceDeclHandler

/* need 'extern int optind' declaration? */
#undef NEED_OPTIND_DECLARATION

/* does expat crash when it sees an initial UTF8 BOM? */
#undef EXPAT_UTF8_BOM_CRASH
