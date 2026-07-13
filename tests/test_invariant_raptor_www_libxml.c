#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Security property: URI scheme and destination validation must reject
 * internal/private network addresses and dangerous URI schemes before
 * any network fetch is attempted. This prevents SSRF attacks.
 *
 * We simulate the URI validation logic that MUST be present in any
 * secure implementation of raptor_www_libxml_fetch or its callers.
 */

/* Simulated URI scheme/destination validator that MUST exist in secure code */
typedef enum {
    URI_ALLOWED = 0,
    URI_BLOCKED_SCHEME = 1,
    URI_BLOCKED_SSRF = 2,
    URI_BLOCKED_INVALID = 3
} uri_validation_result_t;

/*
 * This function represents the security invariant:
 * Any URI passed to the WWW fetch layer MUST be validated.
 * Returns non-zero (blocked) for dangerous URIs, 0 for safe ones.
 */
static uri_validation_result_t validate_uri_security(const char *uri) {
    if (!uri || strlen(uri) == 0) {
        return URI_BLOCKED_INVALID;
    }

    /* Block dangerous URI schemes */
    const char *dangerous_schemes[] = {
        "file://",
        "gopher://",
        "dict://",
        "ftp://",
        "sftp://",
        "tftp://",
        "ldap://",
        "ldaps://",
        "jar:",
        "netdoc:",
        "javascript:",
        "data:",
        "vbscript:",
        NULL
    };

    for (int i = 0; dangerous_schemes[i] != NULL; i++) {
        if (strncasecmp(uri, dangerous_schemes[i], strlen(dangerous_schemes[i])) == 0) {
            return URI_BLOCKED_SCHEME;
        }
    }

    /* Block internal/private network addresses (SSRF targets) */
    const char *ssrf_targets[] = {
        "http://127.",
        "http://localhost",
        "http://0.0.0.0",
        "http://[::1]",
        "http://[::ffff:",
        "http://169.254.",   /* AWS metadata */
        "http://192.168.",
        "http://10.",
        "http://172.16.",
        "http://172.17.",
        "http://172.18.",
        "http://172.19.",
        "http://172.20.",
        "http://172.21.",
        "http://172.22.",
        "http://172.23.",
        "http://172.24.",
        "http://172.25.",
        "http://172.26.",
        "http://172.27.",
        "http://172.28.",
        "http://172.29.",
        "http://172.30.",
        "http://172.31.",
        "https://127.",
        "https://localhost",
        "https://0.0.0.0",
        "https://[::1]",
        "https://169.254.",
        "https://192.168.",
        "https://10.",
        "https://172.16.",
        "https://172.17.",
        "https://172.18.",
        "https://172.19.",
        "https://172.20.",
        "https://172.21.",
        "https://172.22.",
        "https://172.23.",
        "https://172.24.",
        "https://172.25.",
        "https://172.26.",
        "https://172.27.",
        "https://172.28.",
        "https://172.29.",
        "https://172.30.",
        "https://172.31.",
        NULL
    };

    for (int i = 0; ssrf_targets[i] != NULL; i++) {
        if (strncasecmp(uri, ssrf_targets[i], strlen(ssrf_targets[i])) == 0) {
            return URI_BLOCKED_SSRF;
        }
    }

    return URI_ALLOWED;
}

START_TEST(test_ssrf_uri_scheme_blocking)
{
    /* Invariant: Dangerous URI schemes MUST always be blocked before fetch */
    const char *payloads[] = {
        "file:///etc/passwd",
        "file:///etc/shadow",
        "file:///proc/self/environ",
        "file:///windows/system32/drivers/etc/hosts",
        "gopher://internal-host:70/",
        "gopher://127.0.0.1:6379/_FLUSHALL",
        "dict://127.0.0.1:11211/stat",
        "ftp://internal-ftp-server/sensitive-data",
        "sftp://internal-host/etc/passwd",
        "tftp://internal-host/bootfile",
        "ldap://internal-ldap/dc=corp,dc=com",
        "ldaps://internal-ldap/dc=corp,dc=com",
        "jar:file:///tmp/evil.jar!/",
        "netdoc:file:///etc/passwd",
        "javascript:alert(1)",
        "data:text/html,<script>alert(1)</script>",
        "vbscript:msgbox(1)",
        "FILE:///etc/passwd",
        "FILE:///etc/shadow",
        "GOPHER://internal-host:70/",
        "FTP://internal-ftp-server/",
        "Data:text/plain,sensitive",
        NULL
    };

    int num_payloads = 0;
    while (payloads[num_payloads] != NULL) num_payloads++;

    for (int i = 0; i < num_payloads; i++) {
        uri_validation_result_t result = validate_uri_security(payloads[i]);
        ck_assert_msg(result != URI_ALLOWED,
            "SECURITY VIOLATION: Dangerous URI scheme was not blocked: '%s' (result=%d)",
            payloads[i], result);
        ck_assert_msg(result == URI_BLOCKED_SCHEME,
            "SECURITY VIOLATION: URI with dangerous scheme should be blocked as scheme violation: '%s' (result=%d)",
            payloads[i], result);
    }
}
END_TEST

START_TEST(test_ssrf_internal_network_blocking)
{
    /* Invariant: Internal/private network addresses MUST always be blocked */
    const char *payloads[] = {
        "http://127.0.0.1/",
        "http://127.0.0.1:8080/admin",
        "http://127.0.0.1:9200/_cat/indices",
        "http://localhost/",
        "http://localhost:3306/",
        "http://localhost:6379/",
        "http://localhost:11211/",
        "http://0.0.0.0/",
        "http://[::1]/",
        "http://[::1]:8080/",
        "http://169.254.169.254/latest/meta-data/",
        "http://169.254.169.254/latest/meta-data/iam/security-credentials/",
        "http://192.168.1.1/",
        "http://192.168.0.1/admin",
        "http://10.0.0.1/",
        "http://10.0.0.1:8080/internal-api",
        "http://172.16.0.1/",
        "http://172.17.0.1/",
        "http://172.31.255.255/",
        "https://127.0.0.1/",
        "https://localhost/",
        "https://169.254.169.254/",
        "https://192.168.1.1/",
        "https://10.0.0.1/",
        "https://172.16.0.1/",
        NULL
    };

    int num_payloads = 0;
    while (payloads[num_payloads] != NULL) num_payloads++;

    for (int i = 0; i < num_payloads; i++) {
        uri_validation_result_t result = validate_uri_security(payloads[i]);
        ck_assert_msg(result != URI_ALLOWED,
            "SECURITY VIOLATION: Internal network URI was not blocked: '%s' (result=%d)",
            payloads[i], result);
        ck_assert_msg(result == URI_BLOCKED_SSRF,
            "SECURITY VIOLATION: Internal network URI should be blocked as SSRF: '%s' (result=%d)",
            payloads[i], result);
    }
}
END_TEST

START_TEST(test_ssrf_rdf_embedded_uris)
{
    /* Invariant: URIs embedded in RDF documents targeting internal resources MUST be blocked */
    /* These represent URIs that might appear in rdf:resource or rdf:about attributes */
    const char *payloads[] = {
        "http://127.0.0.1:8500/v1/agent/self",          /* Consul */
        "http://127.0.0.1:2379/v2/keys",                 /* etcd */
        "http://127.0.0.1:8080/api/v1/namespaces",       /* Kubernetes */
        "http://169.254.169.254/computeMetadata/v1/",    /* GCP metadata */
        "http://169.254.169.254/latest/user-data",       /* AWS user-data */
        "http://192.168.1.254/",                         /* Router admin */
        "http://10.0.0.1:9090/metrics",                  /* Prometheus */
        "http://10.0.0.1:9200/",                         /* Elasticsearch */
        "http://localhost:27017/",                        /* MongoDB */
        "http://localhost:5432/",                         /* PostgreSQL */
        "http://localhost:3306/",                         /* MySQL */
        "http://localhost:6379/",                         /* Redis */
        "http://localhost:11211/",                        /* Memcached */
        "http://localhost:8161/admin/",                   /* ActiveMQ */
        "http://localhost:4848/",                         /* GlassFish admin */
        "http://localhost:8080/manager/html",             /* Tomcat manager */
        "http://127.0.0.1:4040/",                        /* Spark UI */
        "http://127.0.0.1:8888/",                        /* Jupyter */
        NULL
    };

    int num_payloads = 0;
    while (payloads[num_payloads] != NULL) num_payloads++;

    for (int i = 0; i < num_payloads; i++) {
        uri_validation_result_t result = validate_uri_security(payloads[i]);
        ck_assert_msg(result != URI_ALLOWED,
            "SECURITY VIOLATION: RDF-embedded internal service URI was not blocked: '%s' (result=%d)",
            payloads[i], result);
    }
}
END_TEST

START_TEST(test_ssrf_null_and_empty_uri_handling)
{
    /* Invariant: NULL and empty URIs MUST be rejected safely without crash */
    const char *payloads[] = {
        NULL,
        "",
        " ",
        "\t",
        "\n",
        "\r\n",
    };
    int num_payloads = 6;

    for (int i = 0; i < num_payloads; i++) {
        /* Must not crash and must not return URI_ALLOWED for null/empty */
        uri_validation_result_t result = validate_uri_security(payloads[i]);
        if (payloads[i] == NULL || strlen(payloads[i]) == 0) {
            ck_assert_msg(result == URI_BLOCKED_INVALID,
                "SECURITY VIOLATION: NULL/empty URI should be blocked as invalid (index=%d, result=%d)",
                i, result);
        }
        /* Test passes if we reach here without crashing */
    }
}
END_TEST

START_TEST(test_ssrf_legitimate_uris_allowed)
{
    /* Invariant: Legitimate external HTTP/HTTPS URIs MUST be allowed through */
    const char *payloads[] = {
        "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
        "http://xmlns.com/foaf/0.1/",
        "https://schema.org/Person",
        "http://purl.org/dc/elements/1.1/",
        "https://www.example.com/resource",
        "http://dbpedia.org/resource/Berlin",
        NULL
    };

    int num_payloads = 0;
    while (payloads[num_payloads] != NULL) num_payloads++;

    for (int i = 0; i < num_payloads; i++) {
        uri_validation_result_t result = validate_uri_security(payloads[i]);
        ck_assert_msg(result == URI_ALLOWED,
            "FALSE POSITIVE: Legitimate URI was incorrectly blocked: '%s' (result=%d)",
            payloads[i], result);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security_SSRF_Prevention");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_ssrf_uri_scheme_blocking);
    tcase_add_test(tc_core, test_ssrf_internal_network_blocking);
    tcase_add_test(tc_core, test_ssrf_rdf_embedded_uris);
    tcase_add_test(tc_core, test_ssrf_null_and_empty_uri_handling);
    tcase_add_test(tc_core, test_ssrf_legitimate_uris_allowed);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}