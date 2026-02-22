/*
 * Embedded filesystem data for lwIP HTTP server
 * 
 * This file contains the web pages served by the HTTP server.
 * Files ending in .shtml have SSI tags processed.
 * 
 * Note: This is manually crafted. For larger sites, use makefsdata tool.
 */

#include "lwip/apps/fs.h"
#include <string.h>

/* Forward declaration for file linkage */
struct fsdata_file {
    const struct fsdata_file *next;
    const unsigned char *name;
    const unsigned char *data;
    int len;
    u8_t flags;
#if HTTPD_PRECALCULATED_CHECKSUM
    u16_t chksum_count;
    const struct fsdata_chksum *chksum;
#endif
};

/*---------------------------------------------------------------------------*/
/* File: /index.shtml - Main web page with SSI tags */
/*---------------------------------------------------------------------------*/

static const unsigned char file_name_index_shtml[] = "/index.shtml";

static const unsigned char file_data_index_shtml[] =
"<!DOCTYPE html>\r\n"
"<html>\r\n"
"<head>\r\n"
"<title>Pico 2W Sensor Monitor</title>\r\n"
"<meta http-equiv=\"refresh\" content=\"<!--#refresh-->\">\r\n"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n"
"<style>\r\n"
"body{font-family:Arial,sans-serif;margin:20px;background:#f0f0f0;}\r\n"
"h1{color:#333;text-align:center;}\r\n"
"h2{color:#555;margin-top:0;}\r\n"
".container{max-width:600px;margin:0 auto;}\r\n"
".card{background:white;border-radius:8px;padding:20px;margin:15px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}\r\n"
"table{width:100%;border-collapse:collapse;}\r\n"
"td{padding:10px;border-bottom:1px solid #eee;}\r\n"
"td:first-child{font-weight:bold;color:#666;width:50%;}\r\n"
"td:last-child{text-align:right;font-size:1.1em;}\r\n"
".btn{display:inline-block;padding:12px 24px;margin:5px;border:none;border-radius:4px;cursor:pointer;font-size:1em;text-decoration:none;color:white;}\r\n"
".btn-on{background:#4CAF50;}\r\n"
".btn-off{background:#f44336;}\r\n"
".btn-on:hover{background:#45a049;}\r\n"
".btn-off:hover{background:#da190b;}\r\n"
".led-ON{color:#4CAF50;font-weight:bold;}\r\n"
".led-OFF{color:#999;}\r\n"
"input[type=number]{width:80px;padding:8px;border:1px solid #ddd;border-radius:4px;}\r\n"
"input[type=submit]{padding:8px 16px;background:#2196F3;color:white;border:none;border-radius:4px;cursor:pointer;}\r\n"
"input[type=submit]:hover{background:#1976D2;}\r\n"
".footer{text-align:center;color:#999;margin-top:20px;font-size:0.9em;}\r\n"
".ok{color:#3c763d;}\r\n"
".fail{color:#a94442;}\r\n"
"</style>\r\n"
"</head>\r\n"
"<body>\r\n"
"<div class=\"container\">\r\n"
"<h1>Pico 2W Sensor Monitor</h1>\r\n"
"\r\n"
"<div class=\"card\">\r\n"
"<h2>Power Readings (INA238)</h2>\r\n"
"<table>\r\n"
"<tr><td>Voltage:</td><td><!--#volt--> V</td></tr>\r\n"
"<tr><td>Current:</td><td><!--#curr--> mA</td></tr>\r\n"
"<tr><td>Sensor Status:</td><td class=\"<!--#ina_ok-->\"><!--#ina_ok--></td></tr>\r\n"
"</table>\r\n"
"</div>\r\n"
"\r\n"
"<div class=\"card\">\r\n"
"<h2>Environment (BME280)</h2>\r\n"
"<table>\r\n"
"<tr><td>Temperature:</td><td><!--#temp--> &deg;C</td></tr>\r\n"
"<tr><td>Humidity:</td><td><!--#hum--> %%</td></tr>\r\n"
"<tr><td>Sensor Status:</td><td class=\"<!--#bme_ok-->\"><!--#bme_ok--></td></tr>\r\n"
"</table>\r\n"
"</div>\r\n"
"\r\n"
"<div class=\"card\">\r\n"
"<h2>Payload Control</h2>\r\n"
"<p>LED Status: <span class=\"led-<!--#led-->\"><!--#led--></span></p>\r\n"
"<a href=\"/led?state=on\" class=\"btn btn-on\">Turn ON</a>\r\n"
"<a href=\"/led?state=off\" class=\"btn btn-off\">Turn OFF</a>\r\n"
"</div>\r\n"
"\r\n"
"<div class=\"card\">\r\n"
"<h2>Settings</h2>\r\n"
"<form action=\"/config\" method=\"get\">\r\n"
"<label>Poll Interval: </label>\r\n"
"<input type=\"number\" name=\"interval\" value=\"<!--#poll-->\" min=\"100\" max=\"60000\"> ms\r\n"
"<input type=\"submit\" value=\"Set\">\r\n"
"</form>\r\n"
"</div>\r\n"
"\r\n"
"<div class=\"footer\">\r\n"
"<p>Auto-refresh: <!--#refresh-->s | Poll: <!--#poll-->ms</p>\r\n"
"<p>Pico 2W HostAP I2C Sensor Monitor</p>\r\n"
"</div>\r\n"
"</div>\r\n"
"</body>\r\n"
"</html>\r\n";

/*---------------------------------------------------------------------------*/
/* File: /404.html - Not found page */
/*---------------------------------------------------------------------------*/

static const unsigned char file_name_404_html[] = "/404.html";

static const unsigned char file_data_404_html[] =
"<!DOCTYPE html>\r\n"
"<html>\r\n"
"<head>\r\n"
"<title>404 Not Found</title>\r\n"
"<style>body{font-family:Arial;text-align:center;padding:50px;}</style>\r\n"
"</head>\r\n"
"<body>\r\n"
"<h1>404 - Not Found</h1>\r\n"
"<p>The requested page was not found.</p>\r\n"
"<p><a href=\"/\">Return to Home</a></p>\r\n"
"</body>\r\n"
"</html>\r\n";

/*---------------------------------------------------------------------------*/
/* File system structure */
/*---------------------------------------------------------------------------*/

static const struct fsdata_file file_404_html[] = {{
    NULL,
    file_name_404_html,
    file_data_404_html,
    sizeof(file_data_404_html) - 1,
    FS_FILE_FLAGS_HEADER_INCLUDED,
#if HTTPD_PRECALCULATED_CHECKSUM
    0, NULL
#endif
}};

static const struct fsdata_file file_index_shtml[] = {{
    file_404_html,
    file_name_index_shtml,
    file_data_index_shtml,
    sizeof(file_data_index_shtml) - 1,
    FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_SSI,
#if HTTPD_PRECALCULATED_CHECKSUM
    0, NULL
#endif
}};

#define FS_ROOT file_index_shtml
#define FS_NUMFILES 2

/*---------------------------------------------------------------------------*/
/* File system access functions */
/*---------------------------------------------------------------------------*/

int fs_open_custom(struct fs_file *file, const char *name) {
    const struct fsdata_file *f;
    
    /* Handle root URL */
    if ((name[0] == '/') && (name[1] == '\0')) {
        name = "/index.shtml";
    }
    
    /* Search for file */
    for (f = FS_ROOT; f != NULL; f = f->next) {
        if (!strcmp(name, (const char *)f->name)) {
            file->data = (const char *)f->data;
            file->len = f->len;
            file->index = f->len;
            file->pextension = NULL;
            file->flags = f->flags;
            return 1;  /* File found */
        }
    }
    
    return 0;  /* File not found */
}

void fs_close_custom(struct fs_file *file) {
    /* Nothing to clean up for const data */
    (void)file;
}

int fs_read_custom(struct fs_file *file, char *buffer, int count) {
    /* All data already in memory, nothing to read */
    (void)file;
    (void)buffer;
    (void)count;
    return FS_READ_EOF;
}
