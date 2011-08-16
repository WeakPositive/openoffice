#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,sw))

$(eval $(call gb_Library_add_sdi_headers,sw,sw/sdi/swslots))

$(eval $(call gb_Library_add_precompiled_header,sw,$(SRCDIR)/sw/inc/pch/precompiled_sw))

$(eval $(call gb_Library_set_componentfile,sw,sw/util/sw))

$(eval $(call gb_Library_set_include,sw,\
	-I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/source/filter/inc \
	-I$(SRCDIR)/sw/source/ui/inc \
	-I$(SRCDIR)/sw/inc/pch \
	-I$(SRCDIR)/sw/inc \
	-I$(WORKDIR)/SdiTarget/sw/sdi \
	-I$(WORKDIR)/Misc/sw/ \
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,sw,\
	$$(DEFS) \
	-DSW_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sw,\
	avmedia \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	drawinglayer \
	editeng \
	i18nisolang1 \
	i18nutil \
	icuuc \
	lng \
	sal \
	salhelper \
	sb \
	sfx \
	sot \
	stl \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	ucbhelper \
	utl \
	vbahelper \
	vcl \
	vos3 \
	xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sw,\
	sw/source/core/SwNumberTree/SwNodeNum \
	sw/source/core/SwNumberTree/SwNumberTree \
	sw/source/core/access/acccell \
	sw/source/core/access/acccontext \
	sw/source/core/access/accdoc \
	sw/source/core/access/accembedded \
	sw/source/core/access/accfootnote \
	sw/source/core/access/accframe \
	sw/source/core/access/accframebase\
	sw/source/core/access/accfrmobj \
	sw/source/core/access/accfrmobjmap \
	sw/source/core/access/accfrmobjslist \
	sw/source/core/access/accgraphic \
	sw/source/core/access/accheaderfooter \
	sw/source/core/access/acchyperlink \
	sw/source/core/access/acchypertextdata \
	sw/source/core/access/accmap \
	sw/source/core/access/accnotextframe \
	sw/source/core/access/accpage \
	sw/source/core/access/accpara \
	sw/source/core/access/accportions \
	sw/source/core/access/accpreview \
	sw/source/core/access/accselectionhelper \
	sw/source/core/access/acctable \
	sw/source/core/access/acctextframe \
	sw/source/core/access/parachangetrackinginfo \
	sw/source/core/access/textmarkuphelper \
	sw/source/core/attr/calbck \
	sw/source/core/attr/cellatr \
	sw/source/core/attr/fmtfollowtextflow \
	sw/source/core/attr/fmtwrapinfluenceonobjpos \
	sw/source/core/attr/format \
	sw/source/core/attr/hints \
	sw/source/core/attr/swatrset \
	sw/source/core/bastyp/SwSmartTagMgr \
	sw/source/core/bastyp/bparr \
	sw/source/core/bastyp/breakit \
	sw/source/core/bastyp/calc \
	sw/source/core/bastyp/checkit \
	sw/source/core/bastyp/index \
	sw/source/core/bastyp/init \
	sw/source/core/bastyp/ring \
	sw/source/core/bastyp/swcache \
	sw/source/core/bastyp/swrect \
	sw/source/core/bastyp/swregion \
	sw/source/core/bastyp/swtypes \
	sw/source/core/bastyp/tabcol \
	sw/source/core/crsr/BlockCursor \
	sw/source/core/crsr/bookmrk \
	sw/source/core/crsr/callnk \
	sw/source/core/crsr/crbm \
	sw/source/core/crsr/crossrefbookmark \
	sw/source/core/crsr/crsrsh \
	sw/source/core/crsr/crstrvl \
	sw/source/core/crsr/crstrvl1 \
	sw/source/core/crsr/findattr \
	sw/source/core/crsr/findcoll \
	sw/source/core/crsr/findfmt \
	sw/source/core/crsr/findtxt \
	sw/source/core/crsr/pam \
	sw/source/core/crsr/paminit \
	sw/source/core/crsr/swcrsr \
	sw/source/core/crsr/trvlcol \
	sw/source/core/crsr/trvlfnfl \
	sw/source/core/crsr/trvlreg \
	sw/source/core/crsr/trvltbl \
	sw/source/core/crsr/unocrsr \
	sw/source/core/crsr/viscrs \
	sw/source/core/doc/SwStyleNameMapper \
	sw/source/core/doc/acmplwrd \
	sw/source/core/doc/dbgoutsw \
	sw/source/core/doc/doc \
	sw/source/core/doc/docbasic \
	sw/source/core/doc/docbm \
	sw/source/core/doc/docchart \
	sw/source/core/doc/doccomp \
	sw/source/core/doc/doccorr \
	sw/source/core/doc/docdde \
	sw/source/core/doc/docdesc \
	sw/source/core/doc/docdraw \
	sw/source/core/doc/docedt \
	sw/source/core/doc/docfld \
	sw/source/core/doc/docfly \
	sw/source/core/doc/docfmt \
	sw/source/core/doc/docftn \
	sw/source/core/doc/docglbl \
	sw/source/core/doc/docglos \
	sw/source/core/doc/doclay \
	sw/source/core/doc/docnew \
	sw/source/core/doc/docnum \
	sw/source/core/doc/docredln \
	sw/source/core/doc/docruby \
	sw/source/core/doc/docsort \
	sw/source/core/doc/docstat \
	sw/source/core/doc/doctxm \
	sw/source/core/doc/docxforms \
	sw/source/core/doc/extinput \
	sw/source/core/doc/fmtcol \
	sw/source/core/doc/ftnidx \
	sw/source/core/doc/gctable \
	sw/source/core/doc/htmltbl \
	sw/source/core/doc/lineinfo \
	sw/source/core/doc/list \
	sw/source/core/doc/notxtfrm \
	sw/source/core/doc/number \
	sw/source/core/doc/poolfmt \
	sw/source/core/doc/sortopt \
	sw/source/core/doc/swserv \
	sw/source/core/doc/swstylemanager \
	sw/source/core/doc/tblafmt \
	sw/source/core/doc/tblcpy \
	sw/source/core/doc/tblrwcl \
	sw/source/core/doc/visiturl \
	sw/source/core/docnode/cancellablejob \
	sw/source/core/docnode/finalthreadmanager \
	sw/source/core/docnode/ndcopy \
	sw/source/core/docnode/ndindex \
	sw/source/core/docnode/ndnotxt \
	sw/source/core/docnode/ndnum \
	sw/source/core/docnode/ndsect \
	sw/source/core/docnode/ndtbl \
	sw/source/core/docnode/ndtbl1 \
	sw/source/core/docnode/node \
	sw/source/core/docnode/node2lay \
	sw/source/core/docnode/nodes \
	sw/source/core/docnode/observablethread \
	sw/source/core/docnode/pausethreadstarting \
	sw/source/core/docnode/retrievedinputstreamdata \
	sw/source/core/docnode/retrieveinputstream \
	sw/source/core/docnode/retrieveinputstreamconsumer \
	sw/source/core/docnode/section \
	sw/source/core/docnode/swbaslnk \
	sw/source/core/docnode/swthreadjoiner \
	sw/source/core/docnode/swthreadmanager \
	sw/source/core/docnode/threadlistener \
	sw/source/core/docnode/threadmanager \
	sw/source/core/draw/dcontact \
	sw/source/core/draw/dflyobj \
	sw/source/core/draw/dobjfac \
	sw/source/core/draw/dpage \
	sw/source/core/draw/drawdoc \
	sw/source/core/draw/dview \
	sw/source/core/edit/acorrect \
	sw/source/core/edit/autofmt \
	sw/source/core/edit/edatmisc \
	sw/source/core/edit/edattr \
	sw/source/core/edit/eddel \
	sw/source/core/edit/edfcol \
	sw/source/core/edit/edfld \
	sw/source/core/edit/edfldexp \
	sw/source/core/edit/edfmt \
	sw/source/core/edit/edglbldc \
	sw/source/core/edit/edglss \
	sw/source/core/edit/editsh \
	sw/source/core/edit/edlingu \
	sw/source/core/edit/ednumber \
	sw/source/core/edit/edredln \
	sw/source/core/edit/edsect \
	sw/source/core/edit/edtab \
	sw/source/core/edit/edtox \
	sw/source/core/edit/edundo \
	sw/source/core/edit/edws \
	sw/source/core/except/dbgloop \
	sw/source/core/except/errhdl \
	sw/source/core/fields/authfld \
	sw/source/core/fields/cellfml \
	sw/source/core/fields/chpfld \
	sw/source/core/fields/dbfld \
	sw/source/core/fields/ddefld \
	sw/source/core/fields/ddetbl \
	sw/source/core/fields/docufld \
	sw/source/core/fields/expfld \
	sw/source/core/fields/fldbas \
	sw/source/core/fields/flddat \
	sw/source/core/fields/flddropdown \
	sw/source/core/fields/fldlst \
	sw/source/core/fields/macrofld \
	sw/source/core/fields/postithelper \
	sw/source/core/fields/reffld \
	sw/source/core/fields/scrptfld \
	sw/source/core/fields/tblcalc \
	sw/source/core/fields/textapi \
	sw/source/core/fields/usrfld \
	sw/source/core/frmedt/fecopy \
	sw/source/core/frmedt/fedesc \
	sw/source/core/frmedt/fefly1 \
	sw/source/core/frmedt/feflyole \
	sw/source/core/frmedt/feshview \
	sw/source/core/frmedt/fetab \
	sw/source/core/frmedt/fews \
	sw/source/core/frmedt/tblsel \
	sw/source/core/graphic/grfatr \
	sw/source/core/graphic/ndgrf \
	sw/source/core/layout/anchoreddrawobject \
	sw/source/core/layout/anchoredobject \
	sw/source/core/layout/atrfrm \
	sw/source/core/layout/calcmove \
	sw/source/core/layout/colfrm \
	sw/source/core/layout/dbg_lay \
	sw/source/core/layout/findfrm \
	sw/source/core/layout/flowfrm \
	sw/source/core/layout/fly \
	sw/source/core/layout/flycnt \
	sw/source/core/layout/flyincnt \
	sw/source/core/layout/flylay \
	sw/source/core/layout/flypos \
	sw/source/core/layout/frmtool \
	sw/source/core/layout/ftnfrm \
	sw/source/core/layout/hffrm \
	sw/source/core/layout/layact \
	sw/source/core/layout/laycache \
	sw/source/core/layout/layouter \
	sw/source/core/layout/movedfwdfrmsbyobjpos \
	sw/source/core/layout/newfrm \
	sw/source/core/layout/objectformatter \
	sw/source/core/layout/objectformatterlayfrm \
	sw/source/core/layout/objectformattertxtfrm \
	sw/source/core/layout/objstmpconsiderwrapinfl \
	sw/source/core/layout/pagechg \
	sw/source/core/layout/pagedesc \
	sw/source/core/layout/paintfrm \
	sw/source/core/layout/sectfrm \
	sw/source/core/layout/softpagebreak \
	sw/source/core/layout/sortedobjs \
	sw/source/core/layout/sortedobjsimpl \
	sw/source/core/layout/ssfrm \
	sw/source/core/layout/swselectionlist \
	sw/source/core/layout/tabfrm \
	sw/source/core/layout/trvlfrm \
	sw/source/core/layout/unusedf \
	sw/source/core/layout/virtoutp \
	sw/source/core/layout/wsfrm \
	sw/source/core/objectpositioning/anchoredobjectposition \
	sw/source/core/objectpositioning/ascharanchoredobjectposition \
	sw/source/core/objectpositioning/environmentofanchoredobject \
	sw/source/core/objectpositioning/tocntntanchoredobjectposition \
	sw/source/core/objectpositioning/tolayoutanchoredobjectposition \
	sw/source/core/ole/ndole \
	sw/source/core/para/paratr \
	sw/source/core/sw3io/sw3convert \
	sw/source/core/sw3io/swacorr \
	sw/source/core/swg/SwXMLBlockExport \
	sw/source/core/swg/SwXMLBlockImport \
	sw/source/core/swg/SwXMLBlockListContext \
	sw/source/core/swg/SwXMLSectionList \
	sw/source/core/swg/SwXMLTextBlocks \
	sw/source/core/swg/SwXMLTextBlocks1 \
	sw/source/core/swg/swblocks \
	sw/source/core/table/swnewtable \
	sw/source/core/table/swtable \
	sw/source/core/text/EnhancedPDFExportHelper \
	sw/source/core/text/SwGrammarMarkUp \
	sw/source/core/text/atrstck \
	sw/source/core/text/blink \
	sw/source/core/text/frmcrsr \
	sw/source/core/text/frmform \
	sw/source/core/text/frminf \
	sw/source/core/text/frmpaint \
	sw/source/core/text/guess \
	sw/source/core/text/inftxt \
	sw/source/core/text/itradj \
	sw/source/core/text/itratr \
	sw/source/core/text/itrcrsr \
	sw/source/core/text/itrform2 \
	sw/source/core/text/itrpaint \
	sw/source/core/text/itrtxt \
	sw/source/core/text/noteurl \
	sw/source/core/text/porexp \
	sw/source/core/text/porfld \
	sw/source/core/text/porfly \
	sw/source/core/text/porglue \
	sw/source/core/text/porlay \
	sw/source/core/text/porlin \
	sw/source/core/text/pormulti \
	sw/source/core/text/porref \
	sw/source/core/text/porrst \
	sw/source/core/text/portox \
	sw/source/core/text/portxt \
	sw/source/core/text/redlnitr \
	sw/source/core/text/txtcache \
	sw/source/core/text/txtdrop \
	sw/source/core/text/txtfld \
	sw/source/core/text/txtfly \
	sw/source/core/text/txtfrm \
	sw/source/core/text/txtftn \
	sw/source/core/text/txthyph \
	sw/source/core/text/txtinit \
	sw/source/core/text/txtio \
	sw/source/core/text/txtpaint \
	sw/source/core/text/txttab \
	sw/source/core/text/widorp \
	sw/source/core/text/wrong \
	sw/source/core/tox/tox \
	sw/source/core/tox/toxhlp \
	sw/source/core/tox/txmsrt \
	sw/source/core/txtnode/SwGrammarContact \
	sw/source/core/txtnode/atrfld \
	sw/source/core/txtnode/atrflyin \
	sw/source/core/txtnode/atrftn \
	sw/source/core/txtnode/atrref \
	sw/source/core/txtnode/atrtox \
	sw/source/core/txtnode/chrfmt \
	sw/source/core/txtnode/fmtatr2 \
	sw/source/core/txtnode/fntcache \
	sw/source/core/txtnode/fntcap \
	sw/source/core/txtnode/modeltoviewhelper \
	sw/source/core/txtnode/ndhints \
	sw/source/core/txtnode/ndtxt \
	sw/source/core/txtnode/swfntcch \
	sw/source/core/txtnode/swfont \
	sw/source/core/txtnode/thints \
	sw/source/core/txtnode/txatbase \
	sw/source/core/txtnode/txatritr \
	sw/source/core/txtnode/txtatr2 \
	sw/source/core/txtnode/txtedt \
	sw/source/core/undo/SwRewriter \
	sw/source/core/undo/SwUndoField \
	sw/source/core/undo/SwUndoFmt \
	sw/source/core/undo/SwUndoPageDesc \
	sw/source/core/undo/SwUndoTOXChange \
	sw/source/core/undo/docundo \
	sw/source/core/undo/rolbck \
	sw/source/core/undo/unattr \
	sw/source/core/undo/unbkmk \
	sw/source/core/undo/undel \
	sw/source/core/undo/undobj \
	sw/source/core/undo/undobj1 \
	sw/source/core/undo/undoflystrattr \
	sw/source/core/undo/undraw \
	sw/source/core/undo/unfmco \
	sw/source/core/undo/unins \
	sw/source/core/undo/unmove \
	sw/source/core/undo/unnum \
	sw/source/core/undo/unoutl \
	sw/source/core/undo/unovwr \
	sw/source/core/undo/unredln \
	sw/source/core/undo/unsect \
	sw/source/core/undo/unsort \
	sw/source/core/undo/unspnd \
	sw/source/core/undo/untbl \
	sw/source/core/undo/untblk \
	sw/source/core/unocore/SwXTextDefaults \
	sw/source/core/unocore/TextCursorHelper  \
	sw/source/core/unocore/XMLRangeHelper \
	sw/source/core/unocore/swunohelper \
	sw/source/core/unocore/unobkm \
	sw/source/core/unocore/unochart \
	sw/source/core/unocore/unocoll \
	sw/source/core/unocore/unocrsrhelper \
	sw/source/core/unocore/unodraw \
	sw/source/core/unocore/unoevent \
	sw/source/core/unocore/unoevtlstnr \
	sw/source/core/unocore/unofield \
	sw/source/core/unocore/unoflatpara \
	sw/source/core/unocore/unoframe \
	sw/source/core/unocore/unoftn \
	sw/source/core/unocore/unoidx \
	sw/source/core/unocore/unomap \
	sw/source/core/unocore/unoobj \
	sw/source/core/unocore/unoobj2 \
	sw/source/core/unocore/unoparagraph \
	sw/source/core/unocore/unoport \
	sw/source/core/unocore/unoportenum \
	sw/source/core/unocore/unoprnms \
	sw/source/core/unocore/unoredline \
	sw/source/core/unocore/unoredlines \
	sw/source/core/unocore/unorefmk \
	sw/source/core/unocore/unosect \
	sw/source/core/unocore/unosett \
	sw/source/core/unocore/unosrch \
	sw/source/core/unocore/unostyle \
	sw/source/core/unocore/unotbl  \
	sw/source/core/unocore/unotext \
	sw/source/core/unocore/unotextmarkup \
	sw/source/core/view/pagepreviewlayout \
	sw/source/core/view/printdata \
	sw/source/core/view/vdraw \
	sw/source/core/view/viewimp \
	sw/source/core/view/viewpg \
	sw/source/core/view/viewsh \
	sw/source/core/view/vnew \
	sw/source/core/view/vprint \
	sw/source/filter/ascii/ascatr \
	sw/source/filter/ascii/parasc \
	sw/source/filter/ascii/wrtasc \
	sw/source/filter/basflt/docfact \
	sw/source/filter/basflt/fltini \
	sw/source/filter/basflt/iodetect \
	sw/source/filter/basflt/shellio \
	sw/source/filter/html/SwAppletImpl \
	sw/source/filter/html/css1atr \
	sw/source/filter/html/css1kywd \
	sw/source/filter/html/htmlatr \
	sw/source/filter/html/htmlbas \
	sw/source/filter/html/htmlcss1 \
	sw/source/filter/html/htmlctxt \
	sw/source/filter/html/htmldraw \
	sw/source/filter/html/htmlfld \
	sw/source/filter/html/htmlfldw \
	sw/source/filter/html/htmlfly \
	sw/source/filter/html/htmlflyt \
	sw/source/filter/html/htmlform \
	sw/source/filter/html/htmlforw \
	sw/source/filter/html/htmlftn \
	sw/source/filter/html/htmlgrin \
	sw/source/filter/html/htmlnum \
	sw/source/filter/html/htmlplug \
	sw/source/filter/html/htmlsect \
	sw/source/filter/html/htmltab \
	sw/source/filter/html/htmltabw \
	sw/source/filter/html/parcss1 \
	sw/source/filter/html/svxcss1 \
	sw/source/filter/html/swhtml \
	sw/source/filter/html/wrthtml \
	sw/source/filter/writer/writer \
	sw/source/filter/writer/wrt_fn \
	sw/source/filter/writer/wrtswtbl \
	sw/source/filter/ww1/fltshell \
	sw/source/filter/ww1/w1class \
	sw/source/filter/ww1/w1filter \
	sw/source/filter/ww1/w1par \
	sw/source/filter/ww1/w1sprm \
	sw/source/filter/xml/XMLRedlineImportHelper \
	sw/source/filter/xml/swxml \
	sw/source/filter/xml/wrtxml \
	sw/source/filter/xml/xmlbrsh \
	sw/source/filter/xml/xmlexp \
	sw/source/filter/xml/xmlexpit \
	sw/source/filter/xml/xmlfmt \
	sw/source/filter/xml/xmlfmte \
	sw/source/filter/xml/xmlfonte \
	sw/source/filter/xml/xmlimp \
	sw/source/filter/xml/xmlimpit \
	sw/source/filter/xml/xmlitem \
	sw/source/filter/xml/xmliteme \
	sw/source/filter/xml/xmlitemi \
	sw/source/filter/xml/xmlitemm \
	sw/source/filter/xml/xmlithlp \
	sw/source/filter/xml/xmlitmpr \
	sw/source/filter/xml/xmlmeta \
	sw/source/filter/xml/xmlscript \
	sw/source/filter/xml/xmltble \
	sw/source/filter/xml/xmltbli \
	sw/source/filter/xml/xmltext \
	sw/source/filter/xml/xmltexte \
	sw/source/filter/xml/xmltexti \
	sw/source/ui/app/appenv \
	sw/source/ui/app/apphdl \
	sw/source/ui/app/applab \
	sw/source/ui/app/appopt \
	sw/source/ui/app/docsh \
	sw/source/ui/app/docsh2 \
	sw/source/ui/app/docshdrw \
	sw/source/ui/app/docshini \
	sw/source/ui/app/docst \
	sw/source/ui/app/docstyle \
	sw/source/ui/app/mainwn \
	sw/source/ui/app/swdll \
	sw/source/ui/app/swmodul1 \
	sw/source/ui/app/swmodule \
	sw/source/ui/app/swwait \
	sw/source/ui/cctrl/actctrl \
	sw/source/ui/cctrl/popbox \
	sw/source/ui/cctrl/swlbox \
	sw/source/ui/chrdlg/ccoll \
	sw/source/ui/config/barcfg \
	sw/source/ui/config/caption \
	sw/source/ui/config/cfgitems \
	sw/source/ui/config/dbconfig \
	sw/source/ui/config/fontcfg \
	sw/source/ui/config/modcfg \
	sw/source/ui/config/prtopt \
	sw/source/ui/config/uinums \
	sw/source/ui/config/usrpref \
	sw/source/ui/config/viewopt \
	sw/source/ui/dbui/dbmgr \
	sw/source/ui/dbui/dbtree \
	sw/source/ui/dbui/dbui \
	sw/source/ui/dbui/maildispatcher \
	sw/source/ui/dbui/mailmergechildwindow \
	sw/source/ui/dbui/mailmergehelper \
	sw/source/ui/dbui/mmconfigitem \
	sw/source/ui/dbui/swdbtoolsclient \
	sw/source/ui/dialog/SwSpellDialogChildWindow \
	sw/source/ui/dialog/regionsw \
	sw/source/ui/dialog/swabstdlg \
	sw/source/ui/dialog/swwrtshitem \
	sw/source/ui/dochdl/gloshdl \
	sw/source/ui/dochdl/swdtflvr \
	sw/source/ui/docvw/AnchorOverlayObject \
	sw/source/ui/docvw/AnnotationMenuButton \
	sw/source/ui/docvw/AnnotationWin \
	sw/source/ui/docvw/PostItMgr \
	sw/source/ui/docvw/ShadowOverlayObject \
	sw/source/ui/docvw/SidebarTxtControl \
	sw/source/ui/docvw/SidebarTxtControlAcc \
	sw/source/ui/docvw/SidebarWin \
	sw/source/ui/docvw/SidebarWinAcc \
	sw/source/ui/docvw/edtdd \
	sw/source/ui/docvw/edtwin \
	sw/source/ui/docvw/edtwin2 \
	sw/source/ui/docvw/edtwin3 \
	sw/source/ui/docvw/frmsidebarwincontainer \
	sw/source/ui/docvw/romenu \
	sw/source/ui/docvw/srcedtw \
	sw/source/ui/envelp/envimg \
	sw/source/ui/envelp/labelcfg \
	sw/source/ui/envelp/labimg \
	sw/source/ui/envelp/syncbtn \
	sw/source/ui/fldui/fldmgr \
	sw/source/ui/fldui/fldwrap \
	sw/source/ui/fldui/xfldui \
	sw/source/ui/frmdlg/colex \
	sw/source/ui/frmdlg/colmgr \
	sw/source/ui/frmdlg/frmmgr \
	sw/source/ui/globdoc/globdoc \
	sw/source/ui/index/idxmrk \
	sw/source/ui/index/toxmgr \
	sw/source/ui/lingu/hhcwrp \
	sw/source/ui/lingu/hyp \
	sw/source/ui/lingu/olmenu \
	sw/source/ui/lingu/sdrhhcwrap \
	sw/source/ui/misc/glosdoc \
	sw/source/ui/misc/glshell \
	sw/source/ui/misc/numberingtypelistbox \
	sw/source/ui/misc/redlndlg \
	sw/source/ui/ribbar/conarc \
	sw/source/ui/ribbar/concustomshape \
	sw/source/ui/ribbar/conform \
	sw/source/ui/ribbar/conpoly \
	sw/source/ui/ribbar/conrect \
	sw/source/ui/ribbar/drawbase \
	sw/source/ui/ribbar/dselect \
	sw/source/ui/ribbar/inputwin \
	sw/source/ui/ribbar/tblctrl \
	sw/source/ui/ribbar/tbxanchr \
	sw/source/ui/ribbar/workctrl \
	sw/source/ui/shells/annotsh \
	sw/source/ui/shells/basesh \
	sw/source/ui/shells/beziersh \
	sw/source/ui/shells/drawdlg \
	sw/source/ui/shells/drawsh \
	sw/source/ui/shells/drformsh \
	sw/source/ui/shells/drwbassh \
	sw/source/ui/shells/drwtxtex \
	sw/source/ui/shells/drwtxtsh \
	sw/source/ui/shells/frmsh \
	sw/source/ui/shells/grfsh \
	sw/source/ui/shells/grfshex \
	sw/source/ui/shells/langhelper \
	sw/source/ui/shells/listsh \
	sw/source/ui/shells/mediash \
	sw/source/ui/shells/olesh \
	sw/source/ui/shells/slotadd \
	sw/source/ui/shells/tabsh \
	sw/source/ui/shells/textdrw \
	sw/source/ui/shells/textfld \
	sw/source/ui/shells/textglos \
	sw/source/ui/shells/textidx \
	sw/source/ui/shells/textsh \
	sw/source/ui/shells/textsh1 \
	sw/source/ui/shells/textsh2 \
	sw/source/ui/shells/txtattr \
	sw/source/ui/shells/txtcrsr \
	sw/source/ui/shells/txtnum \
	sw/source/ui/smartmenu/stmenu \
	sw/source/ui/table/chartins \
	sw/source/ui/table/swtablerep \
	sw/source/ui/table/tablemgr \
	sw/source/ui/uiview/formatclipboard \
	sw/source/ui/uiview/pview \
	sw/source/ui/uiview/scroll \
	sw/source/ui/uiview/srcview \
	sw/source/ui/uiview/swcli \
	sw/source/ui/uiview/uivwimp \
	sw/source/ui/uiview/view \
	sw/source/ui/uiview/view0 \
	sw/source/ui/uiview/view1 \
	sw/source/ui/uiview/view2 \
	sw/source/ui/uiview/viewcoll \
	sw/source/ui/uiview/viewdlg \
	sw/source/ui/uiview/viewdlg2 \
	sw/source/ui/uiview/viewdraw \
	sw/source/ui/uiview/viewling \
	sw/source/ui/uiview/viewmdi \
	sw/source/ui/uiview/viewport \
	sw/source/ui/uiview/viewprt \
	sw/source/ui/uiview/viewsrch \
	sw/source/ui/uiview/viewstat \
	sw/source/ui/uiview/viewtab \
	sw/source/ui/uno/RefreshListenerContainer \
	sw/source/ui/uno/SwXDocumentSettings \
	sw/source/ui/uno/SwXFilterOptions \
	sw/source/ui/uno/dlelstnr \
	sw/source/ui/uno/unoatxt \
	sw/source/ui/uno/unodefaults \
	sw/source/ui/uno/unodispatch \
	sw/source/ui/uno/unodoc \
	sw/source/ui/uno/unofreg \
	sw/source/ui/uno/unomailmerge \
	sw/source/ui/uno/unomod \
	sw/source/ui/uno/unomodule \
	sw/source/ui/uno/unotxdoc \
	sw/source/ui/uno/unotxvw \
	sw/source/ui/utlui/attrdesc \
	sw/source/ui/utlui/bookctrl \
	sw/source/ui/utlui/condedit \
	sw/source/ui/utlui/content \
	sw/source/ui/utlui/glbltree \
	sw/source/ui/utlui/gloslst \
	sw/source/ui/utlui/initui \
	sw/source/ui/utlui/navicfg \
	sw/source/ui/utlui/navipi \
	sw/source/ui/utlui/numfmtlb \
	sw/source/ui/utlui/prcntfld \
	sw/source/ui/utlui/shdwcrsr \
	sw/source/ui/utlui/textcontrolcombo \
	sw/source/ui/utlui/tmplctrl \
	sw/source/ui/utlui/uiitems \
	sw/source/ui/utlui/uitool \
	sw/source/ui/utlui/unotools \
	sw/source/ui/utlui/viewlayoutctrl \
	sw/source/ui/utlui/zoomctrl \
	sw/source/ui/web/wdocsh \
	sw/source/ui/web/wformsh \
	sw/source/ui/web/wfrmsh \
	sw/source/ui/web/wgrfsh \
	sw/source/ui/web/wlistsh \
	sw/source/ui/web/wolesh \
	sw/source/ui/web/wtabsh \
	sw/source/ui/web/wtextsh \
	sw/source/ui/web/wview \
	sw/source/ui/wrtsh/delete \
	sw/source/ui/wrtsh/move \
	sw/source/ui/wrtsh/select \
	sw/source/ui/wrtsh/wrtsh1 \
	sw/source/ui/wrtsh/wrtsh2 \
	sw/source/ui/wrtsh/wrtsh3 \
	sw/source/ui/wrtsh/wrtsh4 \
	sw/source/ui/wrtsh/wrtundo \
))

$(eval $(call gb_SdiTarget_SdiTarget,sw/sdi/swslots,sw/sdi/swriter))

$(eval $(call gb_SdiTarget_set_include,sw/sdi/swslots,\
	-I$(SRCDIR)/sw/inc \
	-I$(SRCDIR)/sw/sdi \
	$$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
