#include <cjose/cjose.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

int verify(const char* jwt, const cjose_jwk_t* jwk) {
    cjose_err err;
    cjose_jws_t* jws = cjose_jws_import(jwt, strlen(jwt), &err);
    if (jws == NULL) {
        fprintf(stderr, "Failed to import JWS: %s\n", err.message);
        return 0;
    }

    bool verified = cjose_jws_verify(jws, jwk, &err);
    cjose_jws_release(jws);

    if (!verified) {
        fprintf(stderr, "Failed to verify JWS: %s\n", err.message);
        return 0;
    }

    return 1;
}

cjose_jwk_t** parse_jwks(const char* jwks_json, size_t* jwk_count) {
    json_error_t error;
    json_t* root = json_loads(jwks_json, 0, &error);
    if (!root) {
        fprintf(stderr, "Failed to parse JWKS JSON: %s\n", error.text);
        return NULL;
    }

    json_t* keys = json_object_get(root, "keys");
    if (!json_is_array(keys)) {
        fprintf(stderr, "Invalid JWKS format: 'keys' is not an array\n");
        json_decref(root);
        return NULL;
    }

    *jwk_count = json_array_size(keys);
    cjose_jwk_t** jwks = malloc(*jwk_count * sizeof(cjose_jwk_t*));
    if (!jwks) {
        fprintf(stderr, "Failed to allocate memory for JWKs\n");
        json_decref(root);
        return NULL;
    }

    cjose_err err;
    for (size_t i = 0; i < *jwk_count; i++) {
        json_t* key = json_array_get(keys, i);
        char* key_str = json_dumps(key, JSON_COMPACT);
        jwks[i] = cjose_jwk_import(key_str, strlen(key_str), &err);
        free(key_str);

        if (!jwks[i]) {
            fprintf(stderr, "Failed to import JWK: %s\n", err.message);
            for (size_t j = 0; j < i; j++) {
                cjose_jwk_release(jwks[j]);
            }
            free(jwks);
            json_decref(root);
            return NULL;
        }
    }

    json_decref(root);
    return jwks;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <JWKS or JWK> <JWT>\n", argv[0]);
        return 1;
    }
 
    const char* jwks_or_jwk = argv[1];
    const char* jwt = argv[2];
 
    size_t jwk_count;
    cjose_jwk_t** jwks = parse_jwks(jwks_or_jwk, &jwk_count);
    if (jwks == NULL) {
        cjose_err err;
        cjose_jwk_t* jwk = cjose_jwk_import(jwks_or_jwk, strlen(jwks_or_jwk), &err);
        if (jwk == NULL) {
            fprintf(stderr, "Failed to import JWK: %s\n", err.message);
            return 1;
        }
 
        int result = verify(jwt, jwk);
        cjose_jwk_release(jwk);
        return result ? 0 : 1;
    }
 
    for (size_t i = 0; i < jwk_count; i++) {
        if (verify(jwt, jwks[i])) {
            for (size_t j = 0; j < jwk_count; j++) {
                cjose_jwk_release(jwks[j]);
            }
            free(jwks);
            return 0;
        }
    }
 
    for (size_t i = 0; i < jwk_count; i++) {
        cjose_jwk_release(jwks[i]);
    }
    free(jwks);
 
    fprintf(stderr, "Failed to verify JWT with provided JWKS or JWK\n");
    return 1;
 }
 
