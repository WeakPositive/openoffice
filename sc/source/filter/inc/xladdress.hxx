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

#ifndef SC_XLADDRESS_HXX
#define SC_XLADDRESS_HXX

#include <vector>
#include "address.hxx"

class ScRangeList;
class XclImpStream;
class XclExpStream;

// ============================================================================

/** A 2D cell address struct with Excel column and row indexes. */
struct XclAddress
{
    sal_uInt16          mnCol;
    sal_uInt16          mnRow;

    inline explicit     XclAddress( ScAddress::Uninitialized ) {}
    inline explicit     XclAddress() : mnCol( 0 ), mnRow( 0 ) {}
    inline explicit     XclAddress( sal_uInt16 nCol, sal_uInt16 nRow ) : mnCol( nCol ), mnRow( nRow ) {}

    inline void         Set( sal_uInt16 nCol, sal_uInt16 nRow ) { mnCol = nCol; mnRow = nRow; }

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true ) const;
};

inline bool operator==( const XclAddress& rL, const XclAddress& rR )
{
    return (rL.mnCol == rR.mnCol) && (rL.mnRow == rR.mnRow);
}

inline bool operator<( const XclAddress& rL, const XclAddress& rR )
{
    return (rL.mnCol < rR.mnCol) || ((rL.mnCol == rR.mnCol) && (rL.mnRow < rR.mnRow));
}

inline XclImpStream& operator>>( XclImpStream& rStrm, XclAddress& rXclPos )
{
    rXclPos.Read( rStrm );
    return rStrm;
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclAddress& rXclPos )
{
    rXclPos.Write( rStrm );
    return rStrm;
}

// ----------------------------------------------------------------------------

/** A 2D cell range address struct with Excel column and row indexes. */
struct XclRange
{
    XclAddress          maFirst;
    XclAddress          maLast;

    inline explicit     XclRange( ScAddress::Uninitialized e ) : maFirst( e ), maLast( e ) {}
    inline explicit     XclRange() {}
    inline explicit     XclRange( const XclAddress& rPos ) : maFirst( rPos ), maLast( rPos ) {}
    inline explicit     XclRange( const XclAddress& rFirst, const XclAddress& rLast ) : maFirst( rFirst ), maLast( rLast ) {}
    inline explicit     XclRange( sal_uInt16 nCol1, sal_uInt16 nRow1, sal_uInt16 nCol2, sal_uInt16 nRow2 ) :
                            maFirst( nCol1, nRow1 ), maLast( nCol2, nRow2 ) {}

    inline void         Set( const XclAddress& rFirst, const XclAddress& rLast )
                            { maFirst = rFirst; maLast = rLast; }
    inline void         Set( sal_uInt16 nCol1, sal_uInt16 nRow1, sal_uInt16 nCol2, sal_uInt16 nRow2 )
                            { maFirst.Set( nCol1, nRow1 ); maLast.Set( nCol2, nRow2 ); }

    inline sal_uInt16   GetColCount() const { return maLast.mnCol - maFirst.mnCol + 1; }
    inline sal_uInt16   GetRowCount() const { return maLast.mnRow - maFirst.mnRow + 1; }
    bool                Contains( const XclAddress& rPos ) const;

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true ) const;
};

inline bool operator==( const XclRange& rL, const XclRange& rR )
{
    return (rL.maFirst == rR.maFirst) && (rL.maLast == rR.maLast);
}

inline bool operator<( const XclRange& rL, const XclRange& rR )
{
    return (rL.maFirst < rR.maFirst) || ((rL.maFirst == rR.maFirst) && (rL.maLast < rR.maLast));
}

inline XclImpStream& operator>>( XclImpStream& rStrm, XclRange& rXclRange )
{
    rXclRange.Read( rStrm );
    return rStrm;
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclRange& rXclRange )
{
    rXclRange.Write( rStrm );
    return rStrm;
}

// ----------------------------------------------------------------------------

/** A 2D cell range address list with Excel column and row indexes. */
class XclRangeList : public ::std::vector< XclRange >
{
public:
    inline explicit     XclRangeList() {}

    XclRange            GetEnclosingRange() const;

    void                Read( XclImpStream& rStrm, bool bCol16Bit = true );
    void                Write( XclExpStream& rStrm, bool bCol16Bit = true ) const;
    void                WriteSubList( XclExpStream& rStrm,
                            size_t nBegin, size_t nCount, bool bCol16Bit = true ) const;
};

inline XclImpStream& operator>>( XclImpStream& rStrm, XclRangeList& rXclRanges )
{
    rXclRanges.Read( rStrm );
    return rStrm;
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclRangeList& rXclRanges )
{
    rXclRanges.Write( rStrm );
    return rStrm;
}

// ============================================================================

class XclTracer;

/** Base class for import/export address converters. */
class XclAddressConverterBase
{
public:
    explicit            XclAddressConverterBase( XclTracer& rTracer, const ScAddress& rMaxPos );
    virtual             ~XclAddressConverterBase();

    /** Returns whether the "some columns have been cut" warning box should be shown. */
    inline bool         IsColTruncated() const { return mbColTrunc; }
    /** Returns whether the "some rows have been cut" warning box should be shown. */
    inline bool         IsRowTruncated() const { return mbRowTrunc; }
    /** Returns whether the "some sheets have been cut" warning box should be shown. */
    inline bool         IsTabTruncated() const { return mbTabTrunc; }

    // ------------------------------------------------------------------------

    /** Checks if the passed sheet index is valid.
        @param nScTab  The sheet index to check.
        @param bWarn  true = Sets the internal flag that produces a warning box
            after loading/saving the file, if the sheet index is not valid.
        @return  true = Sheet index in nScTab is valid. */
    bool                CheckScTab( SCTAB nScTab, bool bWarn );

    // ------------------------------------------------------------------------
protected:
    XclTracer&          mrTracer;       /// Tracer for invalid addresses.
    ScAddress           maMaxPos;       /// Default maximum position.
    sal_uInt16          mnMaxCol;       /// Maximum column index, as 16-bit value.
    sal_uInt16          mnMaxRow;       /// Maximum row index, as 16-bit value.
    bool                mbColTrunc;     /// Flag for "columns truncated" warning box.
    bool                mbRowTrunc;     /// Flag for "rows truncated" warning box.
    bool                mbTabTrunc;     /// Flag for "tables truncated" warning box.
};

// ============================================================================

#endif

