#include "nfc_session.h"

#include "mapping.h"

#include <string.h>

static char s_last_uid[kUidLen];

void nfc_session_set_last_uid(const char *uid_hex) {
    strncpy(s_last_uid, uid_hex ? uid_hex : "", kUidLen - 1);
    s_last_uid[kUidLen - 1] = '\0';
}

const char *nfc_session_last_uid() { return s_last_uid; }
