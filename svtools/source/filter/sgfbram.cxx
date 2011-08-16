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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <string.h>
#include <osl/endian.h>
#include <tools/stream.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/color.hxx>
#include <vcl/virdev.hxx>
#include "sgffilt.hxx"
#include "sgfbram.hxx"

/*************************************************************************
|*
|*    operator>>( SvStream&, SgfHeader& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator>>(SvStream& rIStream, SgfHeader& rHead)
{
	rIStream.Read((char*)&rHead.Magic,SgfHeaderSize);
#if defined OSL_BIGENDIAN
	rHead.Magic  =SWAPSHORT(rHead.Magic  );
	rHead.Version=SWAPSHORT(rHead.Version);
	rHead.Typ    =SWAPSHORT(rHead.Typ    );
	rHead.Xsize  =SWAPSHORT(rHead.Xsize  );
	rHead.Ysize  =SWAPSHORT(rHead.Ysize  );
	rHead.Xoffs  =SWAPSHORT(rHead.Xoffs  );
	rHead.Yoffs  =SWAPSHORT(rHead.Yoffs  );
	rHead.Planes =SWAPSHORT(rHead.Planes );
	rHead.SwGrCol=SWAPSHORT(rHead.SwGrCol);
	rHead.OfsLo  =SWAPSHORT(rHead.OfsLo  );
	rHead.OfsHi  =SWAPSHORT(rHead.OfsHi  );
#endif
	return rIStream;
}


/*************************************************************************
|*
|*    SgfHeader::ChkMagic()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_Bool SgfHeader::ChkMagic()
{ return Magic=='J'*256+'J'; }

sal_uInt32 SgfHeader::GetOffset()
{ return sal_uInt32(OfsLo)+0x00010000*sal_uInt32(OfsHi); }


/*************************************************************************
|*
|*    operator>>( SvStream&, SgfEntry& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator>>(SvStream& rIStream, SgfEntry& rEntr)
{
	rIStream.Read((char*)&rEntr.Typ,SgfEntrySize);
#if defined OSL_BIGENDIAN
	rEntr.Typ  =SWAPSHORT(rEntr.Typ  );
	rEntr.iFrei=SWAPSHORT(rEntr.iFrei);
	rEntr.lFreiLo=SWAPSHORT (rEntr.lFreiLo);
	rEntr.lFreiHi=SWAPSHORT (rEntr.lFreiHi);
	rEntr.OfsLo=SWAPSHORT(rEntr.OfsLo);
	rEntr.OfsHi=SWAPSHORT(rEntr.OfsHi);
#endif
	return rIStream;
}

sal_uInt32 SgfEntry::GetOffset()
{ return sal_uInt32(OfsLo)+0x00010000*sal_uInt32(OfsHi); }


/*************************************************************************
|*
|*    operator>>( SvStream&, SgfVector& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator>>(SvStream& rIStream, SgfVector& rVect)
{
	rIStream.Read((char*)&rVect,sizeof(rVect));
#if defined OSL_BIGENDIAN
	rVect.Flag =SWAPSHORT(rVect.Flag );
	rVect.x    =SWAPSHORT(rVect.x    );
	rVect.y    =SWAPSHORT(rVect.y    );
	rVect.OfsLo=SWAPLONG (rVect.OfsLo);
	rVect.OfsHi=SWAPLONG (rVect.OfsHi);
#endif
	return rIStream;
}


/*************************************************************************
|*
|*    operator<<( SvStream&, BmpFileHeader& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator<<(SvStream& rOStream, BmpFileHeader& rHead)
{
#if defined OSL_BIGENDIAN
	rHead.Typ     =SWAPSHORT(rHead.Typ     );
	rHead.SizeLo  =SWAPSHORT(rHead.SizeLo  );
	rHead.SizeHi  =SWAPSHORT(rHead.SizeHi  );
	rHead.Reserve1=SWAPSHORT(rHead.Reserve1);
	rHead.Reserve2=SWAPSHORT(rHead.Reserve2);
	rHead.OfsLo   =SWAPSHORT(rHead.OfsLo   );
	rHead.OfsHi   =SWAPSHORT(rHead.OfsHi   );
#endif
	rOStream.Write((char*)&rHead,sizeof(rHead));
#if defined OSL_BIGENDIAN
	rHead.Typ     =SWAPSHORT(rHead.Typ     );
	rHead.SizeLo  =SWAPSHORT(rHead.SizeLo  );
	rHead.SizeHi  =SWAPSHORT(rHead.SizeHi  );
	rHead.Reserve1=SWAPSHORT(rHead.Reserve1);
	rHead.Reserve2=SWAPSHORT(rHead.Reserve2);
	rHead.OfsLo   =SWAPSHORT(rHead.OfsLo   );
	rHead.OfsHi   =SWAPSHORT(rHead.OfsHi   );
#endif
	return rOStream;
}

void BmpFileHeader::SetSize(sal_uInt32 Size)
{
	SizeLo=sal_uInt16(Size & 0x0000FFFF);
	SizeHi=sal_uInt16((Size & 0xFFFF0000)>>16);
}

void BmpFileHeader::SetOfs(sal_uInt32 Ofs)
{
	OfsLo=sal_uInt16(Ofs & 0x0000FFFF);
	OfsHi=sal_uInt16((Ofs & 0xFFFF0000)>>16);
}

sal_uInt32 BmpFileHeader::GetOfs()
{
	return sal_uInt32(OfsLo)+0x00010000*sal_uInt32(OfsHi);
}

/*************************************************************************
|*
|*    operator<<( SvStream&, BmpInfoHeader& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator<<(SvStream& rOStream, BmpInfoHeader& rInfo)
{
#if defined OSL_BIGENDIAN
	rInfo.Size    =SWAPLONG (rInfo.Size    );
	rInfo.Width   =SWAPLONG (rInfo.Width   );
	rInfo.Hight   =SWAPLONG (rInfo.Hight   );
	rInfo.Planes  =SWAPSHORT(rInfo.Planes  );
	rInfo.PixBits =SWAPSHORT(rInfo.PixBits );
	rInfo.Compress=SWAPLONG (rInfo.Compress);
	rInfo.ImgSize =SWAPLONG (rInfo.ImgSize );
	rInfo.xDpmm   =SWAPLONG (rInfo.xDpmm   );
	rInfo.yDpmm   =SWAPLONG (rInfo.yDpmm   );
	rInfo.ColUsed =SWAPLONG (rInfo.ColUsed );
	rInfo.ColMust =SWAPLONG (rInfo.ColMust );
#endif
	rOStream.Write((char*)&rInfo,sizeof(rInfo));
#if defined OSL_BIGENDIAN
	rInfo.Size    =SWAPLONG (rInfo.Size    );
	rInfo.Width   =SWAPLONG (rInfo.Width   );
	rInfo.Hight   =SWAPLONG (rInfo.Hight   );
	rInfo.Planes  =SWAPSHORT(rInfo.Planes  );
	rInfo.PixBits =SWAPSHORT(rInfo.PixBits );
	rInfo.Compress=SWAPLONG (rInfo.Compress);
	rInfo.ImgSize =SWAPLONG (rInfo.ImgSize );
	rInfo.xDpmm   =SWAPLONG (rInfo.xDpmm   );
	rInfo.yDpmm   =SWAPLONG (rInfo.yDpmm   );
	rInfo.ColUsed =SWAPLONG (rInfo.ColUsed );
	rInfo.ColMust =SWAPLONG (rInfo.ColMust );
#endif
	return rOStream;
}


/*************************************************************************
|*
|*    operator<<( SvStream&, RGBQuad& )
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
SvStream& operator<<(SvStream& rOStream, const RGBQuad& rQuad)
{
	rOStream.Write((char*)&rQuad,sizeof(rQuad));
	return rOStream;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// PcxExpand ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class PcxExpand
{
private:
	sal_uInt16 Count;
	sal_uInt8   Data;
public:
				  PcxExpand() { Count=0; }
	sal_uInt8 GetByte(SvStream& rInp);
};

sal_uInt8 PcxExpand::GetByte(SvStream& rInp)
{
	if (Count>0) {
		Count--;
	} else {
		rInp.Read((char*)&Data,1);
		if ((Data & 0xC0) == 0xC0) {
			Count=(Data & 0x3F) -1;
			rInp.Read((char*)&Data,1);
		}
	}
	return Data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SgfBMapFilter ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


/*************************************************************************
|*
|*    SgfFilterBmp()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_Bool SgfFilterBMap(SvStream& rInp, SvStream& rOut, SgfHeader& rHead, SgfEntry&)
{
	BmpFileHeader  aBmpHead;
	BmpInfoHeader  aBmpInfo;
	sal_uInt16 nWdtInp=(rHead.Xsize+7)/8;  // Breite der Input-Bitmap in Bytes
	sal_uInt16         nWdtOut;            // Breite der Output-Bitmap in Bytes
	sal_uInt16         nColors;            // Anzahl der Farben     (1,16,256)
	sal_uInt16         nColBits;           // Anzahl der Bits/Pixel (2, 4,  8)
    sal_uInt16         i,j,k;              // Spaltenzaehler, Zeilenzaehler, Planezaehler
	sal_uInt16         a,b;                // Hilfsvariable
    sal_uInt8           pl1 = 0,pl2= 0;     // Masken fuer die Planes
    sal_uInt8*          pBuf=NULL;          // Buffer fuer eine Pixelzeile
	PcxExpand      aPcx;
	sal_uLong          nOfs;
	sal_uInt8           cRGB[4];

	if (rHead.Planes<=1) nColBits=1; else nColBits=4; if (rHead.Typ==4) nColBits=8;
	nColors=1<<nColBits;
	nWdtOut=((rHead.Xsize*nColBits+31)/32)*4;
	aBmpHead.Typ='B'+'M'*256;
	aBmpHead.SetOfs(sizeof(aBmpHead)+sizeof(aBmpInfo)+nColors*4);
	aBmpHead.SetSize(aBmpHead.GetOfs()+nWdtOut*rHead.Ysize);
	aBmpHead.Reserve1=0;
	aBmpHead.Reserve2=0;
	aBmpInfo.Size=sizeof(aBmpInfo);
	aBmpInfo.Width=rHead.Xsize;
	aBmpInfo.Hight=rHead.Ysize;
	aBmpInfo.Planes=1;
	aBmpInfo.PixBits=nColBits;
	aBmpInfo.Compress=0;
	aBmpInfo.ImgSize=0;
	aBmpInfo.xDpmm=0;
	aBmpInfo.yDpmm=0;
	aBmpInfo.ColUsed=0;
	aBmpInfo.ColMust=0;
	pBuf=new sal_uInt8[nWdtOut];
	if (!pBuf) return sal_False;       // Fehler: kein Speichel da
	rOut<<aBmpHead<<aBmpInfo;
    memset(pBuf,0,nWdtOut);        // Buffer mit Nullen fuellen

	if (nColors==2)
	{

		rOut<<RGBQuad(0x00,0x00,0x00); // Schwarz
		rOut<<RGBQuad(0xFF,0xFF,0xFF); // Weiss
		nOfs=rOut.Tell();
		for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fuellen
		for (j=0;j<rHead.Ysize;j++) {
			for(i=0;i<nWdtInp;i++) {
				pBuf[i]=aPcx.GetByte(rInp);
			}
			for(i=nWdtInp;i<nWdtOut;i++) pBuf[i]=0;     // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // rueckwaerts schreiben!
			rOut.Write((char*)pBuf,nWdtOut);
		}
	} else if (nColors==16) {
		rOut<<RGBQuad(0x00,0x00,0x00); // Schwarz
		rOut<<RGBQuad(0x24,0x24,0x24); // Grau 80%
		rOut<<RGBQuad(0x49,0x49,0x49); // Grau 60%
		rOut<<RGBQuad(0x92,0x92,0x92); // Grau 40%
		rOut<<RGBQuad(0x6D,0x6D,0x6D); // Grau 30%
		rOut<<RGBQuad(0xB6,0xB6,0xB6); // Grau 20%
		rOut<<RGBQuad(0xDA,0xDA,0xDA); // Grau 10%
		rOut<<RGBQuad(0xFF,0xFF,0xFF); // Weiss
		rOut<<RGBQuad(0x00,0x00,0x00); // Schwarz
		rOut<<RGBQuad(0xFF,0x00,0x00); // Rot
		rOut<<RGBQuad(0x00,0x00,0xFF); // Blau
		rOut<<RGBQuad(0xFF,0x00,0xFF); // Magenta
		rOut<<RGBQuad(0x00,0xFF,0x00); // Gruen
		rOut<<RGBQuad(0xFF,0xFF,0x00); // Gelb
		rOut<<RGBQuad(0x00,0xFF,0xFF); // Cyan
		rOut<<RGBQuad(0xFF,0xFF,0xFF); // Weiss

		nOfs=rOut.Tell();
		for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fuellen
		for (j=0;j<rHead.Ysize;j++) {
			memset(pBuf,0,nWdtOut);
			for(k=0;k<4;k++) {
				if (k==0) {
					pl1=0x10; pl2=0x01;
				} else {
					pl1<<=1; pl2<<=1;
				}
				for(i=0;i<nWdtInp;i++) {
					a=i*4;
					b=aPcx.GetByte(rInp);
					if (b & 0x80) pBuf[a  ]|=pl1;
					if (b & 0x40) pBuf[a  ]|=pl2;
					if (b & 0x20) pBuf[a+1]|=pl1;
					if (b & 0x10) pBuf[a+1]|=pl2;
					if (b & 0x08) pBuf[a+2]|=pl1;
					if (b & 0x04) pBuf[a+2]|=pl2;
					if (b & 0x02) pBuf[a+3]|=pl1;
					if (b & 0x01) pBuf[a+3]|=pl2;
				}
			}
			for(i=nWdtInp*4;i<nWdtOut;i++) pBuf[i]=0;            // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // rueckwaerts schreiben!
			rOut.Write((char*)pBuf,nWdtOut);
		}
	} else if (nColors==256) {
        cRGB[3]=0;                      // der 4. Paletteneintrag fuer BMP
		for (i=0;i<256;i++) {           // Palette kopieren
			rInp.Read((char*)cRGB,3);
			pl1=cRGB[0];                // Rot mit Blau tauschen
			cRGB[0]=cRGB[2];
			cRGB[2]=pl1;
			rOut.Write((char*)cRGB,4);
		}

		nOfs=rOut.Tell();
		for (j=0;j<rHead.Ysize;j++)
            rOut.Write((char*)pBuf,nWdtOut);  // Datei erstmal komplett mit Nullen fuellen
		for (j=0;j<rHead.Ysize;j++) {
			for(i=0;i<rHead.Xsize;i++)
				pBuf[i]=aPcx.GetByte(rInp);
			for(i=rHead.Xsize;i<nWdtOut;i++) pBuf[i]=0;          // noch bis zu 3 Bytes
            rOut.Seek(nOfs+((sal_uLong)rHead.Ysize-j-1L)*(sal_uLong)nWdtOut); // rueckwaerts schreiben!
			rOut.Write((char*)pBuf,nWdtOut);
		}
	}
	delete[] pBuf;
	return sal_True;
}


/*************************************************************************
|*
|*    SgfBMapFilter()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_Bool SgfBMapFilter(SvStream& rInp, SvStream& rOut)
{
	sal_uLong     nFileStart;            // Offset des SgfHeaders. Im allgemeinen 0.
	SgfHeader aHead;
	SgfEntry  aEntr;
	sal_uLong     nNext;
	sal_Bool      bRdFlag=sal_False;         // Grafikentry gelesen ?
	sal_Bool      bRet=sal_False;            // Returncode

	nFileStart=rInp.Tell();
	rInp>>aHead;
	if (aHead.ChkMagic() && (aHead.Typ==SgfBitImag0 || aHead.Typ==SgfBitImag1 ||
							 aHead.Typ==SgfBitImag2 || aHead.Typ==SgfBitImgMo)) {
		nNext=aHead.GetOffset();
		while (nNext && !bRdFlag && !rInp.GetError() && !rOut.GetError()) {
			rInp.Seek(nFileStart+nNext);
			rInp>>aEntr;
			nNext=aEntr.GetOffset();
			if (aEntr.Typ==aHead.Typ) {
				bRdFlag=sal_True;
				switch(aEntr.Typ) {
					case SgfBitImag0:
					case SgfBitImag1:
					case SgfBitImag2:
					case SgfBitImgMo: bRet=SgfFilterBMap(rInp,rOut,aHead,aEntr); break;
				}
			}
		} // while(nNext)
	}
	if (rInp.GetError()) bRet=sal_False;
	return(bRet);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SgfVectFilter ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// Fuer StarDraw Embedded SGF-Vector
long SgfVectXofs=0;
long SgfVectYofs=0;
long SgfVectXmul=0;
long SgfVectYmul=0;
long SgfVectXdiv=0;
long SgfVectYdiv=0;
sal_Bool SgfVectScal=sal_False;

////////////////////////////////////////////////////////////
// Hpgl2SvFarbe ////////////////////////////////////////////
////////////////////////////////////////////////////////////

Color Hpgl2SvFarbe( sal_uInt8 nFarb )
{
	sal_uLong nColor = COL_BLACK;

	switch (nFarb & 0x07) {
		case 0:  nColor=COL_WHITE;        break;
		case 1:  nColor=COL_YELLOW;       break;
		case 2:  nColor=COL_LIGHTMAGENTA; break;
		case 3:  nColor=COL_LIGHTRED;     break;
		case 4:  nColor=COL_LIGHTCYAN;    break;
		case 5:  nColor=COL_LIGHTGREEN;   break;
		case 6:  nColor=COL_LIGHTBLUE;    break;
		case 7:  nColor=COL_BLACK;        break;
	}
	Color aColor( nColor );
	return aColor;
}

/*************************************************************************
|*
|*    SgfFilterVect()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_Bool SgfFilterVect(SvStream& rInp, SgfHeader& rHead, SgfEntry&, GDIMetaFile& rMtf)
{
	VirtualDevice aOutDev;
	SgfVector aVect;
	sal_uInt8      nFarb;
	sal_uInt8      nFrb0=7;
	sal_uInt8      nLTyp;
	sal_uInt8      nOTyp;
	sal_Bool      bEoDt=sal_False;
	sal_Bool      bPDwn=sal_False;
	Point     aP0(0,0);
	Point     aP1(0,0);
	String    Msg;
	sal_uInt16    RecNr=0;

	rMtf.Record(&aOutDev);
	aOutDev.SetLineColor(Color(COL_BLACK));
	aOutDev.SetFillColor(Color(COL_BLACK));

	while (!bEoDt && !rInp.GetError()) {
		rInp>>aVect; RecNr++;
		nFarb=(sal_uInt8) (aVect.Flag & 0x000F);
		nLTyp=(sal_uInt8)((aVect.Flag & 0x00F0) >>4);
		nOTyp=(sal_uInt8)((aVect.Flag & 0x0F00) >>8);
		bEoDt=(aVect.Flag & 0x4000) !=0;
		bPDwn=(aVect.Flag & 0x8000) !=0;

		long x=aVect.x-rHead.Xoffs;
		long y=rHead.Ysize-(aVect.y-rHead.Yoffs);
		if (SgfVectScal) {
			if (SgfVectXdiv==0) SgfVectXdiv=rHead.Xsize;
			if (SgfVectYdiv==0) SgfVectYdiv=rHead.Ysize;
			if (SgfVectXdiv==0) SgfVectXdiv=1;
			if (SgfVectYdiv==0) SgfVectYdiv=1;
			x=SgfVectXofs+ x *SgfVectXmul /SgfVectXdiv;
			y=SgfVectYofs+ y *SgfVectXmul /SgfVectYdiv;
		}
		aP1=Point(x,y);
		if (!bEoDt && !rInp.GetError()) {
			if (bPDwn && nLTyp<=6) {
				switch(nOTyp) {
					case 1: if (nFarb!=nFrb0) {
								switch(rHead.SwGrCol) {
									case SgfVectFarb: aOutDev.SetLineColor(Hpgl2SvFarbe(nFarb)); break;
									case SgfVectGray:                          break;
									case SgfVectWdth:                          break;
								}
							}
							aOutDev.DrawLine(aP0,aP1);            break; // Linie
					case 2:                                       break; // Kreis
					case 3:                                       break; // Text
					case 5: aOutDev.DrawRect(Rectangle(aP0,aP1)); break; // Rechteck (solid)
				}
			}
			aP0=aP1;
			nFrb0=nFarb;
		}
	}
	rMtf.Stop();
	rMtf.WindStart();
	MapMode aMap( MAP_10TH_MM, Point(),
				  Fraction( 1, 4 ), Fraction( 1, 4 ) );
	rMtf.SetPrefMapMode( aMap );
	rMtf.SetPrefSize( Size( (short)rHead.Xsize, (short)rHead.Ysize ) );
	return sal_True;
}


/*************************************************************************
|*
|*    SgfVectFilter()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_Bool SgfVectFilter(SvStream& rInp, GDIMetaFile& rMtf)
{
	sal_uLong     nFileStart;            // Offset des SgfHeaders. Im allgemeinen 0.
	SgfHeader aHead;
	SgfEntry  aEntr;
	sal_uLong     nNext;
	sal_Bool      bRdFlag=sal_False;         // Grafikentry gelesen ?
	sal_Bool      bRet=sal_False;            // Returncode

	nFileStart=rInp.Tell();
	rInp>>aHead;
	if (aHead.ChkMagic() && aHead.Typ==SGF_SIMPVECT) {
		nNext=aHead.GetOffset();
		while (nNext && !bRdFlag && !rInp.GetError()) {
			rInp.Seek(nFileStart+nNext);
			rInp>>aEntr;
			nNext=aEntr.GetOffset();
			if (aEntr.Typ==aHead.Typ) {
				bRet=SgfFilterVect(rInp,aHead,aEntr,rMtf);
			}
		} // while(nNext)
		if (bRdFlag) {
			if (!rInp.GetError()) bRet=sal_True;  // Scheinbar Ok
		}
	}
	return(bRet);
}


/*************************************************************************
|*
|*    SgfFilterPScr()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_Bool SgfFilterPScr(SvStream&, SgfHeader&, SgfEntry&)
{
	return sal_False;  // PostSrcipt wird noch nicht unterstuetzt !
}


/*************************************************************************
|*
|*    CheckSgfTyp()
|*
|*    Beschreibung      Feststellen, um was fuer ein SGF/SGV es sich handelt.
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
sal_uInt8 CheckSgfTyp(SvStream& rInp, sal_uInt16& nVersion)
{
#if OSL_DEBUG_LEVEL > 1 // Recordgroessen checken. Neuer Compiler hat vielleichte anderes Allignment!
	if (sizeof(SgfHeader)!=SgfHeaderSize ||
		sizeof(SgfEntry) !=SgfEntrySize  ||
		sizeof(SgfVector)!=SgfVectorSize ||
		sizeof(BmpFileHeader)!=BmpFileHeaderSize ||
		sizeof(BmpInfoHeader)!=BmpInfoHeaderSize ||
		sizeof(RGBQuad  )!=RGBQuadSize   )  return SGF_DONTKNOW;
#endif

	sal_uLong     nPos;
	SgfHeader aHead;
	nVersion=0;
	nPos=rInp.Tell();
	rInp>>aHead;
	rInp.Seek(nPos);
	if (aHead.ChkMagic()) {
		nVersion=aHead.Version;
		switch(aHead.Typ) {
			case SgfBitImag0:
			case SgfBitImag1:
			case SgfBitImag2:
			case SgfBitImgMo: return SGF_BITIMAGE;
			case SgfSimpVect: return SGF_SIMPVECT;
			case SgfPostScrp: return SGF_POSTSCRP;
			case SgfStarDraw: return SGF_STARDRAW;
			default         : return SGF_DONTKNOW;
		}
	} else {
		return SGF_DONTKNOW;
	}
}
