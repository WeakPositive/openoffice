/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#define copyflnk_mask_width 32
#define copyflnk_mask_height 32
#define copyflnk_mask_x_hot 9
#define copyflnk_mask_y_hot 9
static char copyflnk_mask_bits[] = {
   0xff, 0x01, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00,
   0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00,
   0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00,
   0xff, 0x0f, 0x00, 0x00, 0xff, 0x1f, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00,
   0xff, 0x7f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x01, 0x00,
   0x00, 0xff, 0x03, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00,
   0x00, 0xff, 0x07, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xf1, 0x7f,
   0x00, 0xff, 0xf1, 0x7f, 0x00, 0xe7, 0xf3, 0x7f, 0x00, 0xe0, 0xf3, 0x7f,
   0x00, 0xc0, 0xf7, 0x7f, 0x00, 0xc0, 0xf7, 0x7f, 0x00, 0x80, 0xf7, 0x7f,
   0x00, 0x80, 0xf7, 0x7f, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0xf0, 0x7f,
   0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0x00, 0x00};
