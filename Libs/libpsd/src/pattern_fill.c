/**
 * libpsd - Photoshop file formats (*.psd) decode library
 * Copyright (C) 2004-2007 Graphest Software.
 *
 * libpsd is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: pattern_fill.c, created by Patrick in 2006.06.21, libpsd@graphest.com Exp $
 */

#include "libpsd.h"
#include "psd_system.h"
#include "psd_stream.h"
#include "psd_descriptor.h"


// Pattern fill setting (Photoshop 6.0)
psd_status psd_get_layer_pattern_fill(psd_context * context, psd_layer_record * layer)
{
	psd_layer_pattern_fill * data;
	psd_int length;
	psd_int number_items;
	psd_uint type, key, rootkey;
	psd_uchar keychar[256];
	
	layer->layer_info_type[layer->layer_info_count] = psd_layer_info_type_pattern_fill;
	layer->layer_type = (psd_layer_type)psd_layer_info_type_pattern_fill;

	data = (psd_layer_pattern_fill *)psd_malloc(sizeof(psd_layer_pattern_fill));
	if(data == NULL)
		return psd_status_malloc_failed;
	memset(data, 0, sizeof(psd_layer_pattern_fill));
	layer->layer_info_data[layer->layer_info_count] = (psd_ptr)data;
	layer->layer_info_count ++;

	// Version ( = 16 for Photoshop 6.0)
	if(psd_stream_get_int(context) != 16)
		return psd_status_pattern_fill_unsupport_version;

	// Unicode string: name from classID
	length = psd_stream_get_int(context) * 2;
	psd_stream_get_null(context, length);

	// classID: 4 bytes (length), followed either by string or (if length is zero) 4-
	// byte classID
	length = psd_stream_get_int(context);
	if(length == 0)
		data->id = psd_stream_get_int(context);
	else
		psd_stream_get_null(context, length);

	// Number of items in descriptor
	number_items = psd_stream_get_int(context);

	/***************************************************************************/
	while(number_items--)
	{
		length = psd_stream_get_int(context);
		psd_assert(length == 0);
		if(length == 0)
			rootkey = psd_stream_get_int(context);
		else
		{
			rootkey = 0;
			psd_stream_get(context, keychar, length);
			keychar[length] = 0;
		}
		// Type: OSType key
		type = psd_stream_get_int(context);

		switch(rootkey)
		{
			case 'Scl ':
				psd_assert(type == 'UntF');
				
				// '#Prc' = percent:
				key = psd_stream_get_int(context);
				psd_assert(key == '#Prc');
								
				// Actual value (double)
				data->scale = (psd_int)psd_stream_get_double(context);
				break;

			case 'Ptrn':
				psd_assert(type == 'Objc');
				psd_stream_get_object_pattern_info(&data->pattern_info, context);
				break;

			default:
				psd_assert(0);
				psd_stream_get_object_null(type, context);
				break;
		}
	}

	return psd_status_done;
}
