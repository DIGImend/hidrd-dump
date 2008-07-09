/*
 * Copyright (c) 2008 Nikolai Kondrashov
 *
 * This file is part of digimend-diag.
 *
 * Digimend-diag is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Digimend-diag is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wizardpen-calibrate; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <hid.h>

static const char *
hid_return_str(hid_return rc)
{
    switch (rc) {

#define MAP(_name) \
        case HID_RET_##_name:   \
            return #_name
        MAP(SUCCESS);
        MAP(INVALID_PARAMETER);
        MAP(NOT_INITIALISED);
        MAP(ALREADY_INITIALISED);
        MAP(FAIL_FIND_BUSSES);
        MAP(FAIL_FIND_DEVICES);
        MAP(FAIL_OPEN_DEVICE);
        MAP(DEVICE_NOT_FOUND);
        MAP(DEVICE_NOT_OPENED);
        MAP(DEVICE_ALREADY_OPENED);
        MAP(FAIL_CLOSE_DEVICE);
        MAP(FAIL_CLAIM_IFACE);
        MAP(FAIL_DETACH_DRIVER);
        MAP(NOT_HID_DEVICE);
        MAP(HID_DESC_SHORT);
        MAP(REPORT_DESC_SHORT);
        MAP(REPORT_DESC_LONG);
        MAP(FAIL_ALLOC);
        MAP(OUT_OF_SPACE);
        MAP(FAIL_SET_REPORT);
        MAP(FAIL_GET_REPORT);
        MAP(FAIL_INT_READ);
        MAP(NOT_FOUND);
#undef MAP

        default:
            return "<UNKNOWN>";
    }
}


#define HID_GUARD(_desc, _expr) \
    do {                                                        \
        hid_return  rc;                                         \
                                                                \
        rc = _expr;                                             \
        if (rc != HID_RET_SUCCESS)                              \
        {                                                       \
            fprintf(stderr, "libhid failed to %s: %s (%d)\n",   \
                    _desc, hid_return_str(rc), rc);             \
            exit(1);                                            \
        }                                                       \
    } while (0)


void
hexdump(FILE *out, const void *buf, size_t len)
{
    size_t  pos;

    for (pos = 0; len > 0; len--, buf++, pos++)
    {
        if (pos >= 16)
        {
            fputc('\n', out);
            pos = 0;
        }
        fprintf(out, "%3.2X", (int)*(unsigned char *)buf);
    }

    if (pos != 16)
        fputc('\n', out);
}


int
main(int argc, char **argv)
{
    HIDInterface           *hid;
    HIDInterfaceMatcher     matcher     = {.matcher_fn  = NULL};

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Vendor ID> <Product ID>\n",
                program_invocation_short_name);
        exit(1);
    }
    matcher.vendor_id = (unsigned short)strtol(argv[1], NULL, 16);
    matcher.product_id = (unsigned short)strtol(argv[2], NULL, 16);

    HID_GUARD("initialize", hid_init());

    hid = hid_new_HIDInterface();

    HID_GUARD("force-open the device",
              hid_force_open(hid, 0, &matcher, 3));

    /* Dump report descriptor */
    hexdump(stdout, hid->hid_parser->ReportDesc,
                    hid->hid_parser->ReportDescSize);

    HID_GUARD("close the device", hid_close(hid));

    hid_delete_HIDInterface(&hid);

    HID_GUARD("cleanup", hid_cleanup());

    return 0;
}
