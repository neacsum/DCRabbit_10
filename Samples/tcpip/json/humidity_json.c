/*
   Copyright (c) 2020 Mircea Neacsu.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/*******************************************************************************
        Samples\TcpIp\json\humidity_json.c

        This is an adaptation of a RabbitWeb sample (Samples\tcpip\rabbitweb\humidity.c)
        It replaces the RabbitWeb scripting with sandard JavaScript and shows
        how to use the JSON Bridge for data transfer. It also shows the
        authentication features.  See the pages:

        samples\tcpip\json\pages\humidity_monitor.html

        and

        samples\tcpip\json\pages\humidity_admin.html

        for the corresponding HTML pages.

        When accessing the administration page, note that the username is
        "harpo" and the password is "swordfish".

*******************************************************************************/

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

//max path name
#define SSPEC_MAXNAME  40

/********************************
 * End of configuration section *
 ********************************/


#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"
#use "json.lib"

/*
 * These pages contain the HTML portion of the demonstration.  The first has
 * a status page, while the second has a configuration interface.
 */
#ximport "samples/tcpip/json/pages/humidity_monitor.html"  monitor_html
#ximport "samples/tcpip/json/pages/humidity_admin.html"    admin_html


/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
  SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

// Prototype for user interface functions
int cgi_gethum (HttpState *state);
int cgi_getadmin (HttpState *state);
int cgi_setadmin (HttpState *state);

#define ADMIN 0x02

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
  SSPEC_RESOURCE_XMEMFILE("/index.html", monitor_html),
  SSPEC_RESOURCE_XMEMFILE("/admin/index.html", admin_html),
  SSPEC_RESOURCE_FUNCTION("/gethum.cgi", cgi_gethum),
  SSPEC_RESOURCE_FUNCTION("/getadmin.cgi", cgi_getadmin),
  SSPEC_RESOURCE_FUNCTION("/admin/setadmin.cgi", cgi_setadmin),
SSPEC_RESOURCETABLE_END

int hum;
int hum_alarm;
int alarm_interval;
char alarm_email[50];

/*
  JSON data dictionary.
  Each variable accessed through the JSON libray must be described by a dictionary
  entry. The entry provides the external name of the variable, its type and
  number of elements. If the external name (JSON name) is the same as the
  program name you can use the JSD macro to generate the entry. Otherwise use
  the JSDN macro.
*/
JSD_START
  JSD (hum,             JT_INT, 1, 0),
  JSD (hum_alarm,       JT_INT, 1, 0),
  JSD (alarm_email,     JT_STR, 1, sizeof(alarm_email)),
  JSD (alarm_interval,  JT_INT, 1, 0),
JSD_END;


void main(void)
{
   int userid;

  // Initialize the values
  hum = 50;
  hum_alarm = 75;
  alarm_interval = 60;
  strcpy(alarm_email, "somebody@nowhere.org");

  // Initialize the TCP/IP stack and HTTP server
  // Start network and wait for interface to come up (or error exit).
  sock_init_or_exit(1);
  http_init();
  tcp_reserveport(80);

  // The following line limits access to the "/admin" directory to the admin
  // group.  It also requires basic authentication for the "/admin"
  // directory.
   sspec_addrule("/admin", "Admin", ADMIN, ADMIN, SERVER_ANY,
                 SERVER_AUTH_BASIC, NULL);
                 
  // The following two lines create an "admin" user and adds it to the admin
  // group.
  userid = sauth_adduser("harpo", "swordfish", SERVER_ANY);
  sauth_setusermask(userid, ADMIN, NULL);

  // This drives the HTTP server.
  while(1){
    http_handler();
  }
}

// Retrives data for the 'humidity_monitor' page
int cgi_gethum (HttpState *state)
{
  json_begin (state);
  jsonify (state, &hum);
  json_end (state);
  return 1;
}

// Retrives data for the 'humidity_admin' page
int cgi_getadmin (HttpState *state)
{
  json_begin (state);
  jsonify (state, &hum_alarm);
  jsonify (state, &alarm_email);
  jsonify (state, &alarm_interval);
  json_end (state);
  return 1;
}

// Saves form data for 'humidity_admin' page
int cgi_setadmin (HttpState *state)
{
  url_post (state);
  cgi_redirectto (state, "/admin/index.html");
  printf ("New alarm level is %d\nSending emails to: %s\n",
    hum_alarm, alarm_email);
  return 0;
}



