dnl  
dnl  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
dnl  2011 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl  
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA



AC_DEFUN([GNASH_PKG_CLASSFILE],
[

build_all_as3=yes
classfile=all
classlist=
AC_ARG_WITH(classfile,
  AC_HELP_STRING([--with-classfile=], [Text file with the list of ActionScript classes to build. Default is build all]),
  if test -n ${withval}; then
    classfile="${withval}"
    build_all_as3=no
  else
    classfile=
    AC_MSG_ERROR([No class file was specified! Building everything...])
    build_all_as3=yes
  fi
)

dnl If a classfile was specfied, we're not going to build all 400+ classes
if test x"${classfile}" != x"all"; then
  dnl course if it doesn't exist, all we can do is flag an error and build everything
  if test ! -f ${classfile}; then
    AC_MSG_ERROR([Specified class file ${classfile} doesn't exist! Building everything...])
    build_all_as3=yes
  else
    build_all_as3=no
  fi
fi

if test x"${build_all_as3}" = x"no"; then
  delim=""
  for i in `cat ${classfile}`; do
    echo $i
    classlist="${classlist}${delim} $i"
    delim=","
  done
fi

# If you add or delete a class file, you should also edit this list of makefile conditionals
dnl The core classes
AM_CONDITIONAL(BUILD_UICOMPONENT_AS3, test x"$UIComponent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
dnl if test x"$UIComponent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes"; then
dnl   AC_DEFINE(BUILD_UICOMPONENT_AS3, [], [Build the UICOmponent])
dnl fi

AM_CONDITIONAL(BUILD_INVALIDATIONTYPE_AS3, test x"${InvalidationType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl
dnl The external classes
dnl
AM_CONDITIONAL(BUILD_EXTERNALINTERFACE_AS3, test x"${ExternalInterface_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl The security classes
AM_CONDITIONAL(BUILD_SIGNERTRUSTSETTINGS_AS3, test x"${SignerTrustSettings_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_XMLSIGNATUREVALIDATOR_AS3, test x"${XMLSignatureValidator_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SIGNATURESTATUS_AS3, test x"${SignatureStatus_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IURIDEREFERENCER_AS3, test x"${IURIDereferencer_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_REVOCATIONCHECKSETTINGS_AS3, test x"${RevocationCheckSettings_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl utils/
AM_CONDITIONAL(BUILD_IDATAINPUT_AS3, test x"${IDataInput_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IEXTERNALIZABLE_AS3, test x"${IExternalizable_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BYTEARRAY_AS3, test x"${ByteArray_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PROXY_AS3, test x"${Proxy_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_COMPRESSIONALGORITHM_AS3, test x"${CompressionAlgorithm_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl 
AM_CONDITIONAL(BUILD_IDATAOUTPUT_AS3, test x"${IDataOutput_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ENDIAN_AS3, test x"${Endian_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TIMER_AS3, test x"${Timer_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DICTIONARY_AS3, test x"${Dictionary_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl filesystem
AM_CONDITIONAL(BUILD_FILE_AS3, test x"${File_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FILEMODE_AS3, test x"${FileMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FILESTREAM_AS3, test x"${FileStream_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl Transitions
AM_CONDITIONAL(BUILD_ROTATE_AS3, test x"${Rotate_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PIXELDISSOLVE_AS3, test x"${PixelDissolve_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BLINDS_AS3, test x"${Blinds_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQUEEZE_AS3, test x"${Squeeze_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TRANSITION_AS3, test x"${Transition_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FADE_AS3, test x"${Fade_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TWEEN_AS3, test x"${Tween_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ZOOM_AS3, test x"${Zoom_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IRIS_AS3, test x"${Iris_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FLY_AS3, test x"${Fly_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TWEENEVENT_AS3, test x"${TweenEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PHOTO_AS3, test x"${Photo_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TRANSITIONMANAGER_AS3, test x"${TransitionManager_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_WIPE_AS3, test x"${Wipe_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl managers/
AM_CONDITIONAL(BUILD_IFOCUSMANAGER_AS3, test x"${IFocusManager_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FOCUSMANAGER_AS3, test x"${FocusManager_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IFOCUSMANAGERGROUP_AS3, test x"${IFocusManagerGroup_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STYLEMANAGER_AS3, test x"${StyleManager_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IFOCUSMANAGERCOMPONENT_AS3, test x"${IFocusManagerComponent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl net
AM_CONDITIONAL(BUILD_FILEREFERENCELIST_AS3, test x"${FileReferenceList_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_RESPONDER_AS3, test x"${Responder_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IDYNAMICPROPERTYWRITER_AS3, test x"${IDynamicPropertyWriter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLREQUEST_AS3, test x"${URLRequest_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLREQUESTMETHOD_AS3, test x"${URLRequestMethod_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FILEFILTER_AS3, test x"${FileFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NETSTREAM_AS3, test x"${NetStream_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLREQUESTDEFAULTS_AS3, test x"${URLRequestDefaults_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_OBJECTENCODING_AS3, test x"${ObjectEncoding_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLVARIABLES_AS3, test x"${URLVariables_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLLOADER_AS3, test x"${URLLoader_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NETCONNECTION_AS3, test x"${NetConnection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLREQUESTHEADER_AS3, test x"${URLRequestHeader_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SOCKET_AS3, test x"${Socket_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLLOADERDATAFORMAT_AS3, test x"${URLLoaderDataFormat_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_XMLSOCKET_AS3, test x"${XMLSocket_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FILEREFERENCE_AS3, test x"${FileReference_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SHAREDOBJECTFLUSHSTATUS_AS3, test x"${SharedObjectFlushStatus_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_URLSTREAM_AS3, test x"${URLStream_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IDYNAMICPROPERTYOUTPUT_AS3, test x"${IDynamicPropertyOutput_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SHAREDOBJECT_AS3, test x"${SharedObject_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LOCALCONNECTION_AS3, test x"${LocalConnection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl motion
AM_CONDITIONAL(BUILD_SIMPLEEASE_AS3, test x"${SimpleEase_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TWEENABLES_AS3, test x"${Tweenables_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MOTIONEVENT_AS3, test x"${MotionEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ITWEEN_AS3, test x"${ITween_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CUSTOMEASE_AS3, test x"${CustomEase_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SOURCE_AS3, test x"${Source_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BEZIEREASE_AS3, test x"${BezierEase_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MATRIXTRANSFORMER_AS3, test x"${MatrixTransformer_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BEZIERSEGMENT_AS3, test x"${BezierSegment_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FUNCTIONEASE_AS3, test x"${FunctionEase_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MOTION_AS3, test x"${Motion_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_COLOR_AS3, test x"${Color_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ROTATEDIRECTION_AS3, test x"${RotateDirection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ANIMATOR_AS3, test x"${Animator_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_KEYFRAME_AS3, test x"${Keyframe_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl printing
AM_CONDITIONAL(BUILD_PRINTJOB_AS3, test x"${PrintJob_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PRINTJOBOPTIONS_AS3, test x"${PrintJobOptions_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PRINTJOBORIENTATION_AS3, test x"${PrintJobOrientation_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl geom
AM_CONDITIONAL(BUILD_COLORTRANSFORM_AS3, test x"${ColorTransform_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MATRIX_AS3, test x"${Matrix_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_POINT_AS3, test x"${Point_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TRANSFORM_AS3, test x"${Transform_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_RECTANGLE_AS3, test x"${Rectangle_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl ui
AM_CONDITIONAL(BUILD_CONTEXTMENU_AS3, test x"${ContextMenu_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MOUSE_AS3, test x"${Mouse_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CONTEXTMENUITEM_AS3, test x"${ContextMenuItem_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_KEYLOCATION_AS3, test x"${KeyLocation_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_KEYBOARD_AS3, test x"${Keyboard_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CONTEXTMENUBUILTINITEMS_AS3, test x"${ContextMenuBuiltInItems_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl media
AM_CONDITIONAL(BUILD_SOUNDCHANNEL_AS3, test x"${SoundChannel_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SOUNDTRANSFORM_AS3, test x"${SoundTransform_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SOUNDMIXER_AS3, test x"${SoundMixer_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEO_AS3, test x"${Video_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CAMERA_AS3, test x"${Camera_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MICROPHONE_AS3, test x"${Microphone_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ID3INFO_AS3, test x"${ID3Info_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SOUNDLOADERCONTEXT_AS3, test x"${SoundLoaderContext_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SOUND_AS3, test x"${Sound_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl 
AM_CONDITIONAL(BUILD_ACCESSIBILITY_PROPERTIES_AS3, test x"${AccessibilityProperties_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CHECKBOXACCIMPL_AS3, test x"${CheckBoxAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BUTTONACCIMPL_AS3, test x"${ButtonAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DATAGRIDACCIMPL_AS3, test x"${DataGridAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_RADIOBUTTONACCIMPL_AS3, test x"${RadioButtonAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ACCIMPL_AS3, test x"${AccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TILELISTACCIMPL_AS3, test x"${TileListAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LISTACCIMPL_AS3, test x"${ListAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_COMBOBOXACCIMPL_AS3, test x"${ComboBoxAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_UICOMPONENTACCIMPL_AS3, test x"${UIComponentAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LABELBUTTONACCIMPL_AS3, test x"${LabelButtonAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ACCESSIBILITY_AS3, test x"${Accessibility_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SELECTABLELISTACCIMPL_AS3, test x"${SelectableListAccImpl_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl events
AM_CONDITIONAL(BUILD_SQLEVENT_AS3, test x"${SQLEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_OUTPUTPROGRESSEVENT_AS3, test x"${OutputProgressEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_HTTPSTATUSEVENT_AS3, test x"${HTTPStatusEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BROWSERINVOKEEVENT_AS3, test x"${BrowserInvokeEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DRMAUTHENTICATEEVENT_AS3, test x"${DRMAuthenticateEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SECURITYERROREVENT_AS3, test x"${SecurityErrorEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLUPDATEEVENT_AS3, test x"${SQLUpdateEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MOUSEEVENT_AS3, test x"${MouseEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DATAEVENT_AS3, test x"${DataEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TIMEREVENT_AS3, test x"${TimerEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_HTMLUNCAUGHTSCRIPTEXCEPTIONEVENT_AS3, test x"${HTMLUncaughtScriptExceptionEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEWINDOWDISPLAYSTATEEVENT_AS3, test x"${NativeWindowDisplayStateEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ASYNCERROREVENT_AS3, test x"${AsyncErrorEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STATUSEVENT_AS3, test x"${StatusEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_EVENTDISPATCHER_AS3, test x"${EventDispatcher_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FOCUSEVENT_AS3, test x"${FocusEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_EVENTPHASE_AS3, test x"${EventPhase_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IMEEVENT_AS3, test x"${IMEEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FILELISTEVENT_AS3, test x"${FileListEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SYNCEVENT_AS3, test x"${SyncEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DRMSTATUSEVENT_AS3, test x"${DRMStatusEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTEVENT_AS3, test x"${TextEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_EVENT_AS3, test x"${Event_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PROGRESSEVENT_AS3, test x"${ProgressEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CONTEXTMENUEVENT_AS3, test x"${ContextMenuEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IOERROREVENT_AS3, test x"${IOErrorEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_INVOKEEVENT_AS3, test x"${InvokeEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEWINDOWBOUNDSEVENT_AS3, test x"${NativeWindowBoundsEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IEVENTDISPATCHER_AS3, test x"${IEventDispatcher_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ERROREVENT_AS3, test x"${ErrorEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FULLSCREENEVENT_AS3, test x"${FullScreenEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DRMERROREVENT_AS3, test x"${DRMErrorEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEDRAGEVENT_AS3, test x"${NativeDragEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NETSTATUSEVENT_AS3, test x"${NetStatusEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_KEYBOARDEVENT_AS3, test x"${KeyboardEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLERROREVENT_AS3, test x"${SQLErrorEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCREENMOUSEEVENT_AS3, test x"${ScreenMouseEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ACTIVITYEVENT_AS3, test x"${ActivityEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl filters
AM_CONDITIONAL(BUILD_BEVELFILTER_AS3, test x"${BevelFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BITMAPFILTERQUALITY_AS3, test x"${BitmapFilterQuality_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DROPSHADOWFILTER_AS3, test x"${DropShadowFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_GLOWFILTER_AS3, test x"${GlowFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CONVOLUTIONFILTER_AS3, test x"${ConvolutionFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BITMAPFILTERTYPE_AS3, test x"${BitmapFilterType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_GRADIENTGLOWFILTER_AS3, test x"${GradientGlowFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BLURFILTER_AS3, test x"${BlurFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DISPLACEMENTMAPFILTER_AS3, test x"${DisplacementMapFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BITMAPFILTER_AS3, test x"${BitmapFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_GRADIENTBEVELFILTER_AS3, test x"${GradientBevelFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DISPLACEMENTMAPFILTERMODE_AS3, test x"${DisplacementMapFilterMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_COLORMATRIXFILTER_AS3, test x"${ColorMatrixFilter_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl video
AM_CONDITIONAL(BUILD_AUTOLAYOUTEVENT_AS3, test x"${AutoLayoutEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FLVPLAYBACKCAPTIONING_AS3, test x"${FLVPlaybackCaptioning_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SOUNDEVENT_AS3, test x"${SoundEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LAYOUTEVENT_AS3, test x"${LayoutEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_INCMANAGER_AS3, test x"${INCManager_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEOERROR_AS3, test x"${VideoError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FLVPLAYBACK_AS3, test x"${FLVPlayback_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEOALIGN_AS3, test x"${VideoAlign_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NCMANAGER_AS3, test x"${NCManager_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEOPLAYER_AS3, test x"${VideoPlayer_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CAPTIONCHANGEEVENT_AS3, test x"${CaptionChangeEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEOSTATE_AS3, test x"${VideoState_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NCMANAGERNATIVE_AS3, test x"${NCManagerNative_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEOSCALEMODE_AS3, test x"${VideoScaleMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CAPTIONTARGETEVENT_AS3, test x"${CaptionTargetEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEOEVENT_AS3, test x"${VideoEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IVPEVENT_AS3, test x"${IVPEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_METADATAEVENT_AS3, test x"${MetadataEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CUEPOINTTYPE_AS3, test x"${CuePointType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_VIDEOPROGRESSEVENT_AS3, test x"${VideoProgressEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SKINERROREVENT_AS3, test x"${SkinErrorEvent_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl text
AM_CONDITIONAL(BUILD_TEXTFIELDAUTOSIZE_AS3, test x"${TextFieldAutoSize_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTRENDERER_AS3, test x"${TextRenderer_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CSMSETTINGS_AS3, test x"${CSMSettings_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTDISPLAYMODE_AS3, test x"${TextDisplayMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTLINEMETRICS_AS3, test x"${TextLineMetrics_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_GRIDFITTYPE_AS3, test x"${GridFitType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ANTIALIASTYPE_AS3, test x"${AntiAliasType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STYLESHEET_AS3, test x"${StyleSheet_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTFIELDTYPE_AS3, test x"${TextFieldType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FONTSTYLE_AS3, test x"${FontStyle_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTFIELD_AS3, test x"${TextField_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTCOLORTYPE_AS3, test x"${TextColorType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FONTTYPE_AS3, test x"${FontType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FONT_AS3, test x"${Font_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STATICTEXT_AS3, test x"${StaticText_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTFORMAT_AS3, test x"${TextFormat_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTSNAPSHOT_AS3, test x"${TextSnapshot_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTFORMATALIGN_AS3, test x"${TextFormatAlign_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl data
AM_CONDITIONAL(BUILD_SQLTRANSACTIONLOCKTYPE_AS3, test x"${SQLTransactionLockType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLSCHEMARESULT_AS3, test x"${SQLSchemaResult_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLTRIGGERSCHEMA_AS3, test x"${SQLTriggerSchema_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLTABLESCHEMA_AS3, test x"${SQLTableSchema_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLCONNECTION_AS3, test x"${SQLConnection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLINDEXSCHEMA_AS3, test x"${SQLIndexSchema_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLRESULT_AS3, test x"${SQLResult_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLSCHEMA_AS3, test x"${SQLSchema_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLCOLLATIONTYPE_AS3, test x"${SQLCollationType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLVIEWSCHEMA_AS3, test x"${SQLViewSchema_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLCOLUMNNAMESTYLE_AS3, test x"${SQLColumnNameStyle_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLMODE_AS3, test x"${SQLMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ENCRYPTEDLOCALSTORE_AS3, test x"${EncryptedLocalStore_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLSTATEMENT_AS3, test x"${SQLStatement_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLCOLUMNSCHEMA_AS3, test x"${SQLColumnSchema_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl system
AM_CONDITIONAL(BUILD_IMECONVERSIONMODE_AS3, test x"${IMEConversionMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IME_AS3, test x"${IME_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SECURITYPANEL_AS3, test x"${SecurityPanel_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CAPABILITIES_AS3, test x"${Capabilities_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_APPLICATIONDOMAIN_AS3, test x"${ApplicationDomain_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LOADERCONTEXT_AS3, test x"${LoaderContext_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SYSTEM_AS3, test x"${System_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SECURITYDOMAIN_AS3, test x"${SecurityDomain_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SECURITY_AS3, test x"${Security_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl errors
AM_CONDITIONAL(BUILD_INVALIDSWFERROR_AS3, test x"${InvalidSWFError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STACKOVERFLOWERROR_AS3, test x"${StackOverflowError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IOERROR_AS3, test x"${IOError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MEMORYERROR_AS3, test x"${MemoryError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLERROR_AS3, test x"${SQLError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ILLEGALOPERATIONERROR_AS3, test x"${IllegalOperationError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_EOFERROR_AS3, test x"${EOFError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCRIPTTIMEOUTERROR_AS3, test x"${ScriptTimeoutError_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SQLERROROPERATION_AS3, test x"${SQLErrorOperation_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl desktop
AM_CONDITIONAL(BUILD_UPDATER_AS3, test x"${Updater_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEDRAGMANAGER_AS3, test x"${NativeDragManager_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEAPPLICATION_AS3, test x"${NativeApplication_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEDRAGACTIONS_AS3, test x"${NativeDragActions_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CLIPBOARD_AS3, test x"${Clipboard_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NOTIFICATIONTYPE_AS3, test x"${NotificationType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CLIPBOARDFORMATS_AS3, test x"${ClipboardFormats_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CLIPBOARDTRANSFERMODE_AS3, test x"${ClipboardTransferMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEDRAGOPTIONS_AS3, test x"${NativeDragOptions_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DOCKICON_AS3, test x"${DockIcon_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SYSTEMTRAYICON_AS3, test x"${SystemTrayIcon_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ICON_AS3, test x"${Icon_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_INTERACTIVEICON_AS3, test x"${InteractiveIcon_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl controls
AM_CONDITIONAL(BUILD_PROGRESSBARDIRECTION_AS3, test x"${ProgressBarDirection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCROLLPOLICY_AS3, test x"${ScrollPolicy_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SLIDER_AS3, test x"${Slider_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTINPUT_AS3, test x"${TextInput_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TEXTAREA_AS3, test x"${TextArea_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SELECTABLELIST_AS3, test x"${SelectableList_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LABEL_AS3, test x"${Label_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCROLLBAR_AS3, test x"${ScrollBar_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BASEBUTTON_AS3, test x"${BaseButton_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCROLLBARDIRECTION_AS3, test x"${ScrollBarDirection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_RADIOBUTTONGROUP_AS3, test x"${RadioButtonGroup_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_COMBOBOX_AS3, test x"${ComboBox_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LABELBUTTON_AS3, test x"${LabelButton_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NUMERICSTEPPER_AS3, test x"${NumericStepper_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_UISCROLLBAR_AS3, test x"${UIScrollBar_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PROGRESSBARMODE_AS3, test x"${ProgressBarMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DATAGRID_AS3, test x"${DataGrid_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PROGRESSBAR_AS3, test x"${ProgressBar_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BUTTON_AS3, test x"${Button_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_COLORPICKER_AS3, test x"${ColorPicker_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CHECKBOX_AS3, test x"${CheckBox_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_TILELIST_AS3, test x"${TileList_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LIST_AS3, test x"${List_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BUTTONLABELPLACEMENT_AS3, test x"${ButtonLabelPlacement_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SLIDERDIRECTION_AS3, test x"${SliderDirection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_RADIOBUTTON_AS3, test x"${RadioButton_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl xml
AM_CONDITIONAL(BUILD_XMLNODE_AS3, test x"${XMLNode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_XMLNODETYPE_AS3, test x"${XMLNodeType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_XMLDOCUMENT_AS3, test x"${XMLDocument_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl display
AM_CONDITIONAL(BUILD_FRAMELABEL_AS3, test x"${FrameLabel_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCREEN_AS3, test x"${Screen_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BITMAP_AS3, test x"${Bitmap_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SWFVERSION_AS3, test x"${SWFVersion_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STAGEALIGN_AS3, test x"${StageAlign_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STAGEDISPLAYSTATE_AS3, test x"${StageDisplayState_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_IBITMAPDRAWABLE_AS3, test x"${IBitmapDrawable_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEWINDOWTYPE_AS3, test x"${NativeWindowType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEMENUITEM_AS3, test x"${NativeMenuItem_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_INTERPOLATIONMETHOD_AS3, test x"${InterpolationMethod_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_INTERACTIVEOBJECT_AS3, test x"${InteractiveObject_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BITMAPDATA_AS3, test x"${BitmapData_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCENE_AS3, test x"${Scene_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STAGE_AS3, test x"${Stage_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STAGESCALEMODE_AS3, test x"${StageScaleMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SPRITE_AS3, test x"${Sprite_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SPREADMETHOD_AS3, test x"${SpreadMethod_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SIMPLEBUTTON_AS3, test x"${SimpleButton_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEWINDOWRESIZE_AS3, test x"${NativeWindowResize_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEWINDOW_AS3, test x"${NativeWindow_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_AVM1MOVIE_AS3, test x"${AVM1Movie_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_GRADIENTTYPE_AS3, test x"${GradientType_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DISPLAYOBJECT_AS3, test x"${DisplayObject_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_GRAPHICS_AS3, test x"${Graphics_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_CAPSSTYLE_AS3, test x"${CapsStyle_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BITMAPDATACHANNEL_AS3, test x"${BitmapDataChannel_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MOVIECLIP_AS3, test x"${MovieClip_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STAGEQUALITY_AS3, test x"${StageQuality_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_PIXELSNAPPING_AS3, test x"${PixelSnapping_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_ACTIONSCRIPTVERSION_AS3, test x"${ActionScriptVersion_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LOADERINFO_AS3, test x"${LoaderInfo_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LINESCALEMODE_AS3, test x"${LineScaleMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_JOINTSTYLE_AS3, test x"${JointStyle_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEWINDOWDISPLAYSTATE_AS3, test x"${NativeWindowDisplayState_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_FOCUSDIRECTION_AS3, test x"${FocusDirection_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_BLENDMODE_AS3, test x"${BlendMode_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEMENU_AS3, test x"${NativeMenu_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_MORPHSHAPE_AS3, test x"${MorphShape_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SHAPE_AS3, test x"${Shape_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NATIVEWINDOWINITOPTIONS_AS3, test x"${NativeWindowInitOptions_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_LOADER_AS3, test x"${Loader_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DISPLAYOBJECTCONTAINER_AS3, test x"${DisplayObjectContainer_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl containers
AM_CONDITIONAL(BUILD_BASESCROLLPANE_AS3, test x"${BaseScrollPane_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_UILOADER_AS3, test x"${UILoader_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_SCROLLPANE_AS3, test x"${ScrollPane_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl sampler
AM_CONDITIONAL(BUILD_SAMPLE_AS3, test x"${Sample_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_STACKFRAME_AS3, test x"${StackFrame_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_NEWOBJECTSAMPLE_AS3, test x"${NewObjectSample_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_DELETEOBJECTSAMPLE_AS3, test x"${DeleteObjectSample_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

dnl html
AM_CONDITIONAL(BUILD_HTMLWINDOWCREATEOPTIONS_AS3, test x"${HTMLWindowCreateOptions_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_HTMLLOADER_AS3, test x"${HTMLLoader_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_HTMLHISTORYITEM_AS3, test x"${HTMLHistoryItem_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_HTMLPDFCAPABILITY_AS3, test x"${HTMLPDFCapability_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")
AM_CONDITIONAL(BUILD_HTMLHOST_AS3, test x"${HTMLHost_as3}" = x"yes" -o x"${build_all_as3}" = x"yes")

CLASSLIST=${classlist}
AC_SUBST(CLASSLIST)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
