#include "json.h"

#include <stdlib.h>
#include <string.h>

#include "strbuf.h"
#include "util.h"

char *json_escape_alloc(const char *text)
{
    StrBuf sb;
    sb_init(&sb);
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) {
        switch (*p) {
        case '"':
            sb_puts(&sb, "\\\"");
            break;
        case '\\':
            sb_puts(&sb, "\\\\");
            break;
        case '\b':
            sb_puts(&sb, "\\b");
            break;
        case '\f':
            sb_puts(&sb, "\\f");
            break;
        case '\n':
            sb_puts(&sb, "\\n");
            break;
        case '\r':
            sb_puts(&sb, "\\r");
            break;
        case '\t':
            sb_puts(&sb, "\\t");
            break;
        default:
            if (*p < 0x20) {
                sb_printf(&sb, "\\u%04x", *p);
            } else {
                sb_putc(&sb, (char)*p);
            }
            break;
        }
    }
    return sb_take(&sb);
}

static const char *skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }
    return p;
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

static void put_utf8(StrBuf *sb, unsigned cp)
{
    if (cp < 0x80) {
        sb_putc(sb, (char)cp);
    } else if (cp < 0x800) {
        sb_putc(sb, (char)(0xC0 | (cp >> 6)));
        sb_putc(sb, (char)(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        sb_putc(sb, (char)(0xE0 | (cp >> 12)));
        sb_putc(sb, (char)(0x80 | ((cp >> 6) & 0x3F)));
        sb_putc(sb, (char)(0x80 | (cp & 0x3F)));
    } else {
        sb_putc(sb, (char)(0xF0 | (cp >> 18)));
        sb_putc(sb, (char)(0x80 | ((cp >> 12) & 0x3F)));
        sb_putc(sb, (char)(0x80 | ((cp >> 6) & 0x3F)));
        sb_putc(sb, (char)(0x80 | (cp & 0x3F)));
    }
}

static int read_hex4(const char *p, unsigned *out)
{
    unsigned value = 0;
    for (int i = 0; i < 4; i++) {
        int digit = hex_value(p[i]);
        if (digit < 0) {
            return 0;
        }
        value = (value << 4) | (unsigned)digit;
    }
    *out = value;
    return 1;
}

char *json_get_string_alloc(const char *json, const char *key)
{
    size_t key_len = strlen(key);
    char *needle = xmalloc(key_len + 3);
    needle[0] = '"';
    memcpy(needle + 1, key, key_len);
    needle[key_len + 1] = '"';
    needle[key_len + 2] = '\0';

    const char *p = strstr(json, needle);
    free(needle);
    if (!p) {
        return NULL;
    }
    p += key_len + 2;
    p = skip_ws(p);
    if (*p != ':') {
        return NULL;
    }
    p++;
    p = skip_ws(p);
    if (*p != '"') {
        return NULL;
    }
    p++;

    StrBuf sb;
    sb_init(&sb);
    while (*p && *p != '"') {
        if (*p != '\\') {
            sb_putc(&sb, *p++);
            continue;
        }
        p++;
        switch (*p) {
        case '"':
            sb_putc(&sb, '"');
            p++;
            break;
        case '\\':
            sb_putc(&sb, '\\');
            p++;
            break;
        case '/':
            sb_putc(&sb, '/');
            p++;
            break;
        case 'b':
            sb_putc(&sb, '\b');
            p++;
            break;
        case 'f':
            sb_putc(&sb, '\f');
            p++;
            break;
        case 'n':
            sb_putc(&sb, '\n');
            p++;
            break;
        case 'r':
            sb_putc(&sb, '\r');
            p++;
            break;
        case 't':
            sb_putc(&sb, '\t');
            p++;
            break;
        case 'u': {
            unsigned cp;
            if (!read_hex4(p + 1, &cp)) {
                p++;
                break;
            }
            p += 5;
            if (cp >= 0xD800 && cp <= 0xDBFF && p[0] == '\\' &&
                p[1] == 'u') {
                unsigned low;
                if (read_hex4(p + 2, &low) && low >= 0xDC00 &&
                    low <= 0xDFFF) {
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                    p += 6;
                }
            }
            put_utf8(&sb, cp);
            break;
        }
        default:
            p++;
            break;
        }
    }
    return sb_take(&sb);
}
