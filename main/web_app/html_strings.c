/*
 * html_strings.c
 *
 *  Created on: 18 но€б. 2018 г.
 *      Author: lvr_laptop
 */
#include "httpmg_server/include/mongoose.h"

const char *doorPage =
      "HTTP/1.0 200 OK\r\n"
      "Connection: close\r\n"
      "Content-Type: text/html\r\n"
      "\r\n"
      "<html>"
		  "<head>"
		  	  "<meta charset=\"utf-8\">"
		  "</head>"
		  "<body>"
		  	  "<table border=\"1\" rules=\"all\">"
		  	  	  "<tr>"
		  	  	  	  "<td>Attribute</td>"
		  	  	  	  "<td>Value</td>"
		  	  	  	  "<td>Units</td>"
		  	  	  "</tr>"
		  	  	  "<tr>"
		  	  	  	  "<td>Battery</td>"
		  	  	  	  "<td>%d</td>"
		  	  	  	  "<td>[percents]</td>"
		  	  	  "</tr>"
		  	  	  "<tr>"
		  	  	  	  "<td>Door</td>"
		  	  	  	  "<td>%s</td>"
		  	  	  	  "<td>[state]</td>"
		  	  	  "</tr>"
				  "<tr>"
					  "<td>Lock</td>"
		   	   	   	  "<td> <form> <button type=\"submit\" name=\"lock\" value=\"O\"> open </button> </form> </td>"
					  "<td>[input]</td>"
				  "</tr>"
		  	  "</table>"
		  "</body"
      "</html>\r\n";

bool IsReqOpen(struct mg_str* qstr)
{
	for (uint16_t i = 0; i < qstr->len; i++)
	{
		if (qstr->p[i] == '=')
		{
			if (qstr->p[i + 1] == 'O')
				{return true;}
		}
	}
	return false;
}
