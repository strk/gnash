// TextField_as.cpp:  ActionScript "TextField" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "TextField_as.h"

#include "namedStrings.h"
#include "TextField.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "TextFormat_as.h"
#include "MovieClip.h"
#include "NativeFunction.h"
#include "GnashNumeric.h"
#include "Movie.h"
#include "fontlib.h"
#include "utf8.h"
#include "Font.h"

namespace gnash {

// Forward declarations
namespace {
    const char* autoSizeValueName(TextField::AutoSize val);
    TextField::AutoSize parseAutoSize(const std::string& val);

    void attachPrototypeProperties(as_object& proto);
    void attachTextFieldStaticMembers(as_object& o);
    void attachTextFieldInterface(as_object& o);

    as_value textfield_createTextField(const fn_call& fn);
    
    as_value textfield_variable(const fn_call& fn);
    as_value textfield_setTextFormat(const fn_call& fn);
    as_value textfield_getTextFormat(const fn_call& fn);
    as_value textfield_setNewTextFormat(const fn_call& fn);
    as_value textfield_getNewTextFormat(const fn_call& fn);
    as_value textfield_getDepth(const fn_call& fn);
    as_value textfield_getFontList(const fn_call& fn);
    as_value textfield_removeTextField(const fn_call& fn);
    as_value textfield_replaceSel(const fn_call& fn);
    as_value textfield_replaceText(const fn_call& fn);
    as_value textfield_password(const fn_call& fn);
    as_value textfield_ctor(const fn_call& fn);
    as_value textfield_multiline(const fn_call& fn);
    as_value textfield_scroll(const fn_call& fn);
    as_value textfield_maxscroll(const fn_call& fn);
    as_value textfield_maxhscroll(const fn_call& fn);
    as_value textfield_maxChars(const fn_call& fn);
    as_value textfield_bottomScroll(const fn_call& fn);
    as_value textfield_hscroll(const fn_call& fn);
    as_value textfield_htmlText(const fn_call& fn);
    as_value textfield_restrict(const fn_call& fn);
    as_value textfield_background(const fn_call& fn);
    as_value textfield_border(const fn_call& fn);
    as_value textfield_backgroundColor(const fn_call& fn);
    as_value textfield_borderColor(const fn_call& fn);
    as_value textfield_text(const fn_call& fn);
    as_value textfield_textColor(const fn_call& fn);
    as_value textfield_embedFonts(const fn_call& fn);
    as_value textfield_autoSize(const fn_call& fn);
    as_value textfield_type(const fn_call& fn);
    as_value textfield_wordWrap(const fn_call& fn);
    as_value textfield_html(const fn_call& fn);
    as_value textfield_selectable(const fn_call& fn);
    as_value textfield_length(const fn_call& fn);
    as_value textfield_textWidth(const fn_call& fn);
    as_value textfield_textHeight(const fn_call& fn);
}

as_object*
createTextFieldObject(Global_as& gl)
{
    as_value tf(getMember(gl, NSV::CLASS_TEXT_FIELD));
    as_function* ctor = tf.to_function();
    if (!ctor) return 0;
    fn_call::Args args;
    as_environment env(getVM(gl));
    return constructInstance(*ctor, env, args);
}

/// This provides the prototype and static methods for TextField.
//
/// For SWF5 there is initially no prototype, for SWF6+ there is a 
/// limited prototype. This is changed later on instantiation of a
/// TextField.
void
textfield_class_init(as_object& where, const ObjectURI& uri)
{

    Global_as& gl = getGlobal(where);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(&textfield_ctor, proto);

    attachTextFieldInterface(*proto);
    attachTextFieldStaticMembers(*cl);
             
    where.init_member(uri, cl, as_object::DefaultFlags);

    // ASSetPropFlags is called on the TextField class.
    as_object* null = 0;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, cl, null, 131);
}

void
registerTextFieldNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(textfield_replaceSel, 104, 100);
    vm.registerNative(textfield_getTextFormat, 104, 101);
    vm.registerNative(textfield_setTextFormat, 104, 102);
    vm.registerNative(textfield_removeTextField, 104, 103);
    vm.registerNative(textfield_getNewTextFormat, 104, 104);
    vm.registerNative(textfield_setNewTextFormat, 104, 105);
    vm.registerNative(textfield_getDepth, 104, 106);
    vm.registerNative(textfield_replaceText, 104, 107);

    vm.registerNative(textfield_createTextField, 104, 200);
    vm.registerNative(textfield_getFontList, 104, 201);
}


namespace {

void
attachPrototypeProperties(as_object& o)
{
    // SWF6 or higher
    const int swf6Flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;

    // The following properties should only be attached to the prototype
    // on first textfield creation.
    o.init_property(NSV::PROP_TEXT_WIDTH,
            textfield_textWidth, textfield_textWidth);
    o.init_property(NSV::PROP_TEXT_HEIGHT,
            textfield_textHeight, textfield_textHeight);

    Global_as& gl = getGlobal(o);

    as_function* getset = gl.createFunction(textfield_variable);
    o.init_property("variable", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_background);
    o.init_property("background", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_text);
    o.init_property("text", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_backgroundColor);
    o.init_property("backgroundColor", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_border);
    o.init_property("border", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_borderColor);
    o.init_property("borderColor", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_textColor);
    o.init_property("textColor", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_embedFonts);
    o.init_property("embedFonts", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_autoSize);
    o.init_property("autoSize", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_type);
    o.init_property("type", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_wordWrap);
    o.init_property("wordWrap", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_html);
    o.init_property("html", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_selectable);
    o.init_property("selectable", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_length);
    o.init_property("length", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_maxscroll);
    o.init_property("maxscroll", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_maxhscroll);
    o.init_property("maxhscroll", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_maxChars);
    o.init_property("maxChars", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_bottomScroll);
    o.init_property("bottomScroll", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_scroll);
    o.init_property("scroll", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_hscroll);
    o.init_property("hscroll", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_restrict);
    o.init_property("restrict", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_multiline);
    o.init_property("multiline", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_password);
    o.init_property("password", *getset, *getset, swf6Flags);
    getset = gl.createFunction(textfield_htmlText);
    o.init_property("htmlText", *getset, *getset, swf6Flags);
}


/// This is in fact a property of MovieClip, but it is more a TextField
/// function, as its major number (104) in the native table shows.
as_value
textfield_createTextField(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);
    
    // name, depth, x, y, width, height
    if (fn.nargs < 6) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField called with %d args, "
            "expected 6 - returning undefined"), fn.nargs);
        );
        return as_value();
    }

    const std::string& name = fn.arg(0).to_string();
    const int depth = toInt(fn.arg(1), getVM(fn));
    const int x = toInt(fn.arg(2), getVM(fn));
    const int y = toInt(fn.arg(3), getVM(fn));
    
    int width = toInt(fn.arg(4), getVM(fn));
    if (width < 0) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField: negative width (%d)"
            " - reverting sign"), width);
        );
        width = -width;
    }

    int height = toInt(fn.arg(5), getVM(fn));
    if ( height < 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField: negative height (%d)"
            " - reverting sign"), height);
        );
        height = -height;
    }
    // Set textfield bounds
    SWFRect bounds(0, 0, pixelsToTwips(width), pixelsToTwips(height));

    // Tests in actionscript.all/TextField.as show that this function must:
    //  1. Call "new _global.TextField()" (which takes care of
    //     assigning properties to the prototype).
    //  2. Make that object into a TextField and put it on the display list.
    as_object* obj = createTextFieldObject(getGlobal(fn));

    if (!obj) return as_value();

    DisplayObject* tf = new TextField(obj, ptr, bounds);

    VM& vm = getVM(fn);

    // Give name and mark as dynamic
    tf->set_name(getURI(vm, name));
    tf->setDynamic();

    // Set _x and _y
    SWFMatrix matrix;
    matrix.set_translation(pixelsToTwips(x), pixelsToTwips(y));
    // update caches (although shouldn't be needed as we only set translation)
    tf->setMatrix(matrix, true); 

    ptr->addDisplayListObject(tf, depth);

    if (getSWFVersion(fn) > 7) return as_value(obj);
    return as_value(); 
}

as_value
textfield_background(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (fn.nargs == 0) {
        return as_value(ptr->getDrawBackground());
    }
    else {
        ptr->setDrawBackground(toBool(fn.arg(0), getVM(fn)));
    }

    return as_value();
}

as_value
textfield_border(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (fn.nargs == 0) {
        return as_value(ptr->getDrawBorder());
    }
    else {
        ptr->setDrawBorder(toBool(fn.arg(0), getVM(fn)));
    }

    return as_value();
}

as_value
textfield_backgroundColor(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (fn.nargs == 0) {
        return as_value(ptr->getBackgroundColor().toRGB());
    }
    else {
        rgba newColor;
        newColor.parseRGB(static_cast<boost::uint32_t>(toInt(fn.arg(0), getVM(fn))));
        ptr->setBackgroundColor(newColor);
    }

    return as_value();
}

as_value
textfield_borderColor(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (fn.nargs == 0) {
        return as_value(ptr->getBorderColor().toRGB());
    }
    else {
        rgba newColor;
        newColor.parseRGB(static_cast<boost::uint32_t>(toNumber(fn.arg(0), getVM(fn))));
        ptr->setBorderColor(newColor);
    }

    return as_value();
}

    
as_value
textfield_textColor(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs) {
        // Getter
        return as_value(ptr->getTextColor().toRGB());
    }

    // Setter
    rgba newColor;
    newColor.parseRGB(static_cast<boost::uint32_t>(toNumber(fn.arg(0), getVM(fn))));
    ptr->setTextColor(newColor);

    return as_value();
}

as_value
textfield_embedFonts(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs) {
        // Getter
        return as_value(ptr->getEmbedFonts());
    }

    // Setter
    ptr->setEmbedFonts(toBool(fn.arg(0), getVM(fn)));
    return as_value();
}

as_value
textfield_wordWrap(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (fn.nargs == 0) {
        return as_value(ptr->doWordWrap());
    }
    else {
        ptr->setWordWrap(toBool(fn.arg(0), getVM(fn)));
    }

    return as_value();
}

as_value
textfield_html(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (fn.nargs == 0) {
        return as_value(ptr->doHtml());
    }
    else {
        ptr->setHtml( toBool(fn.arg(0), getVM(fn)) );
    }

    return as_value();
}

as_value
textfield_selectable(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->isSelectable());
    }
    else // setter
    {
        ptr->setSelectable( toBool(fn.arg(0), getVM(fn)) );
    }

    return as_value();
}

as_value
textfield_length(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if ( fn.nargs == 0 ) // getter
    {
        const std::string& s = ptr->get_text_value();
        return as_value(s.length()); // TOCHECK: utf-8 ?
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set length property of TextField %s"),
            ptr->getTarget());
        );
    }

    return as_value();
}

as_value
textfield_textHeight(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if ( fn.nargs == 0 ) // getter
    {
        // Return the height, in pixels, of the text as laid out.
        // (I.e. the actual text content, not our defined
        // bounding box.)
        //
        // In local coords.  Verified against Macromedia Flash.
        return as_value(twipsToPixels(ptr->getTextBoundingBox().height()));

    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set read-only %s property of TextField "
                "%s"), "textHeight", ptr->getTarget());
        );
    }

    return as_value();
}

as_value
textfield_textWidth(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if ( fn.nargs == 0 ) // getter
    {
        // Return the width, in pixels, of the text as laid out.
        // (I.e. the actual text content, not our defined
        // bounding box.)
        //
        // In local coords.  Verified against Macromedia Flash.
        return as_value(twipsToPixels(ptr->getTextBoundingBox().width()));

    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set read-only %s property of TextField %s"),
            "textWidth", ptr->getTarget());
        );
    }

    return as_value();
}

as_value
textfield_autoSize(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if ( fn.nargs == 0 ) // getter
    {
        return autoSizeValueName(ptr->getAutoSize());
    }
    else // setter
    {
        const as_value& arg = fn.arg(0);
        if (arg.is_bool()) {
            if (toBool(arg, getVM(fn))) {
                // True equates to left, every other bool to none.
                ptr->setAutoSize(TextField::AUTOSIZE_LEFT);
            }
            else {
                ptr->setAutoSize(TextField::AUTOSIZE_NONE);
            }
        }
        else {
            std::string strval = arg.to_string();
            TextField::AutoSize val = parseAutoSize(strval);
            ptr->setAutoSize(val);
        }
    }

    return as_value();
}

as_value
textfield_type(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs) {
        // getter
        return ptr->typeValueName(ptr->getType());
    }

    // setter
    const as_value& arg = fn.arg(0);
    std::string strval = arg.to_string();
    TextField::TypeValue val = ptr->parseTypeValue(strval);

    IF_VERBOSE_ASCODING_ERRORS(
        if (val == TextField::typeInvalid) {
            log_aserror(_("Invalid value given to TextField.type: %s"), strval);
        }
    );
    ptr->setType(val);
    return as_value();
}


as_value
textfield_variable(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs)
    {
        // Getter
        const std::string& varName = text->getVariableName();
        // An empty variable name returns null.
        if (varName.empty()) {
            as_value null;
            null.set_null();
            return null;
        }
        return as_value(varName);
    }

    // Setter
    const as_value& varName = fn.arg(0);
    if (varName.is_undefined() || varName.is_null()) {
        text->set_variable_name("");
    }
    else text->set_variable_name(varName.to_string());

    return as_value();

}


as_value
textfield_getDepth(const fn_call& fn)
{
    // Unlike MovieClip.getDepth this works only for TextFields.
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);
    const int n = text->get_depth();
    return as_value(n);
}

as_value
textfield_getFontList(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);
    UNUSED(text);

    LOG_ONCE(log_unimpl(_("TextField.getFontList()")));

    return as_value();
}

as_value
textfield_getNewTextFormat(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);
    UNUSED(text);

    LOG_ONCE(log_unimpl(_("TextField.getNewTextFormat()")));

    return as_value();
}


// This is a bit of a messy compromise. We call the TextFormat ctor (this
// is necessary because prototype properties are not attached until that is
// done). Then we access the relay object directly. This is because there
// aren't enough known parameters to the TextFormat constructor to handle
// all the values, and it isn't tested properly.
as_value
textfield_getTextFormat(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    Global_as& gl = getGlobal(fn);
    as_function* ctor = getMember(gl, NSV::CLASS_TEXT_FORMAT).to_function();

    if (!ctor) return as_value();

    fn_call::Args args;
    as_object* textformat = constructInstance(*ctor, fn.env(), args);
    TextFormat_as* tf;

    if (!isNativeType(textformat, tf)) {
        return as_value();
    }

    tf->alignSet(text->getTextAlignment());
    tf->sizeSet(text->getFontHeight());
    tf->indentSet(text->getIndent());
    tf->blockIndentSet(text->getBlockIndent());
    tf->leadingSet(text->getLeading());
    tf->leftMarginSet(text->getLeftMargin());
    tf->rightMarginSet(text->getRightMargin());
    tf->colorSet(text->getTextColor());
    tf->underlinedSet(text->getUnderlined());

    const Font* font = text->getFont();
    if (font)
    {
        tf->fontSet(font->name());
        tf->italicSet(font->isItalic());
        tf->boldSet(font->isBold());
    }

    // TODO: add font color and some more

    LOG_ONCE(
        log_unimpl(_("TextField.getTextFormat() discards URL, target, "
                     "tabStops, bullet and display")));

    return as_value(textformat);
}

as_value
textfield_setTextFormat(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("TextField.setTextFormat(%s) : %s"), ss.str(),
                _("missing arg")));
        return as_value();
    }
    else if (fn.nargs > 1) {
        LOG_ONCE(
            std::stringstream ss; fn.dump_args(ss);
            log_unimpl(_("TextField.setTextFormat(%s) : args past the first are "
                         "unhandled by Gnash"), ss.str());
        );
    }

    // Note there are three overloads for this functions, each taking
    // a TextFormat as the last argument.
    // TODO: handle the cases where text indices are passed as first
    // and second arguments.

    TextFormat_as* tf;
    if (!isNativeType(toObject(fn.arg(0), getVM(fn)), tf)) {

        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("TextField.setTextFormat(%s) : %s"), ss.str(), 
                _("first argument is not a TextFormat"))
            );
        return as_value();
    }

    if (tf->font()) {
        const std::string& fontName = *tf->font();
        if (!fontName.empty()) {
            bool bold = tf->bold() ? *tf->bold() : false;
            bool italic = tf->italic() ? *tf->italic() : false;

            // NOTE: should query movie-private font lib, not global-shared one
            Movie* mi = text->get_root();
            assert(mi);
            const movie_definition* md = mi->definition();
            assert(md);
            Font* f = md->get_font(fontName, bold, italic);
            if ( ! f ) f = fontlib::get_font(fontName, bold, italic);
            text->setFont( f );
        }
    }

    // TODO: add font color and some more
    text->setTextFormat(*tf);
    return as_value();

}

as_value
textfield_setNewTextFormat(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);
    UNUSED(text);

    LOG_ONCE(log_unimpl(_("TextField.setNewTextFormat(), we'll delegate "
                          "to setTextFormat")));
    return textfield_setTextFormat(fn);
}

as_value
textfield_password(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs)
    {
        // Getter
        return as_value(text->password());
    }
    // Setter
    text->password(toBool(fn.arg(0), getVM(fn)));
    return as_value();
}

as_value
textfield_multiline(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs) {
        // Getter
        return as_value(text->multiline());
    }
    // Setter
    text->multiline(toBool(fn.arg(0), getVM(fn)));
    return as_value();
}

as_value
textfield_restrict(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs) {
        // Getter
        if (text->isRestrict()) {
            return as_value(text->getRestrict());
        } else {
            as_value null;
            null.set_null();
            return null;
        }
    }
    // Setter
    text->setRestrict(fn.arg(0).to_string());
    return as_value();
}

as_value
textfield_bottomScroll(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);
    UNUSED(text);

    LOG_ONCE(log_unimpl(_("TextField.bottomScroll is not complete")));


    if (!fn.nargs)
    {
        // Getter
        return as_value(1 + text->getBottomScroll());
    }
    // Setter
    //text->setBottomScroll(int(toNumber(fn.arg(0), getVM(fn)))); READ-ONLY

    return as_value();
}

as_value
textfield_maxhscroll(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);
    UNUSED(text);

    LOG_ONCE(log_unimpl(_("TextField.maxhscroll is not complete")));


    if (!fn.nargs)
    {
        // Getter
        return as_value(text->getMaxHScroll());
    }
    // Setter
    //text->setMaxHScroll(int(toNumber(fn.arg(0), getVM(fn)))); READ-ONLY

    return as_value();
}

/// TextField.maxChars().
//
/// This does not limit the length of the text, but rather the
/// number of DisplayObjects that can be entered in the TextField.
//
/// Returns null when the value is 0.
as_value
textfield_maxChars(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs)
    {
        boost::int32_t maxChars = text->maxChars();
        if (maxChars == 0)
        {
            as_value null;
            null.set_null();
            return null;
        }
        return as_value(maxChars);
    }
    // Setter
    text->maxChars(toInt(fn.arg(0), getVM(fn)));
    return as_value();
}

as_value
textfield_text(const fn_call& fn)
{
    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);
    if (!fn.nargs)
    {
        // Getter
        //
        // FIXME: should return text without HTML tags.
        return as_value(ptr->get_text_value());
    }

    // Setter
    const int version = getSWFVersion(fn);
    ptr->setTextValue(
            utf8::decodeCanonicalString(fn.arg(0).to_string(), version));

    return as_value();
}

as_value
textfield_htmlText(const fn_call& fn)
{

    TextField* ptr = ensure<IsDisplayObject<TextField> >(fn);
    if (!fn.nargs) {
        // Getter
        return as_value(ptr->get_htmltext_value());
    }

    // Setter
    const int version = getSWFVersion(fn);
    
    ptr->setTextValue(utf8::decodeCanonicalString(fn.arg(0).to_string(),
                version));

    return as_value();
}

/// TextField.replaceSel(newText)
//
/// Replaces the current selection with the new text, setting both
/// begin and end of the selection to one after the inserted text.
/// If an empty string is passed, SWF8 erases the selection; SWF7 and below
/// is a no-op.
/// If no argument is passed, this is a no-op.
as_value
textfield_replaceSel(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("TextField.replaceSel(%s) requires exactly one "
                          "argument"), os.str());
        );
        return as_value();
    }

    const std::string& replace = fn.arg(0).to_string();

    /// Do nothing if text is empty and version less than 8.
    const int version = getSWFVersion(fn);
    if (version < 8 && replace.empty()) return as_value();

    text->replaceSelection(replace);

    return as_value();
}

as_value
textfield_scroll(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);
    UNUSED(text);

    if (!fn.nargs)
    {
        // Getter
        return as_value(1 + text->getScroll());
    }
    // Setter
    text->setScroll(int(toNumber(fn.arg(0), getVM(fn))) - 1); 

    return as_value();
}

as_value
textfield_hscroll(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    LOG_ONCE(log_unimpl(_("TextField._hscroll is not complete")));

    if (!fn.nargs)
    {
        // Getter
        return as_value(text->getHScroll());
    }
    // Setter
    text->setHScroll(int(toNumber(fn.arg(0), getVM(fn))));

    return as_value();
}

as_value
textfield_maxscroll(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    LOG_ONCE(log_unimpl(_("TextField.maxscroll is not complete")));

    if (!fn.nargs) {
        // Getter
        return as_value(text->getMaxScroll());
    }

    return as_value();
}

as_value
textfield_replaceText(const fn_call& fn)
{
    using std::string;
    using std::wstring;

    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    if ( fn.nargs < 3 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("TextField.replaceText() called with less than 3 args"));
        )
        return as_value();
    }

    int userEnd = toInt(fn.arg(1), getVM(fn));
    if ( userEnd < 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("TextField.replaceText(%s): negative endIndex"
                      " - doing nothing"), ss.str());
        );
        return as_value();
    }

    wstring::size_type start = toInt(fn.arg(0), getVM(fn));
    wstring::size_type end = userEnd;

    int version = getSWFVersion(fn);

    // TODO: check if it's possible for SWF6 to use this function
    //       and if it is whether to_string should be to_string
    //       (affects the way undefined values are considered)
    const wstring& replacement =
        utf8::decodeCanonicalString(fn.arg(2).to_string(), version);

    // TODO: drop this round uf8 encoding and decoding by exposing
    //       a TextField::getTextValue ?
    const wstring& subject =
        utf8::decodeCanonicalString(text->get_text_value(), version);

    if ( start > subject.length() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("TextField.replaceText(%s): beginIndex out of range"
                      " - doing nothing"), ss.str());
        );
        return as_value();
    }


    // TODO: use STL proper
    wstring newstring;
    if ( start ) newstring = subject.substr(0, start);
    newstring.append(replacement);

    if ( end > subject.length() )
    {
        //log_aserror...
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("TextField.replaceText(%s): endIndex out of range"
                      " - taking as end of string"), ss.str());
        );
    }
    else
    {
        newstring.append(subject.substr(end));
    }

    // TODO: check if we should really be updating registered variables
    text->setTextValue(newstring);

    return as_value();
}

as_value
textfield_removeTextField(const fn_call& fn)
{
    TextField* text = ensure<IsDisplayObject<TextField> >(fn);

    text->removeTextField();

    LOG_ONCE(log_debug("TextField.removeTextField() TESTING"));

    return as_value();
}


/// This is called for 'new TextField()'
//
/// Note that MovieClip.createTextField must call this function (or anything
/// that replaces it).
//
/// Tests in actionscript.all/TextField.as show that this constructor:
///     1. Adds properties to the prototype.
///     2. Removes array typing.
///     3. Removes any Relay.
///     4. Does not produce a DisplayObject.
///     5. Operates on a 'this' pointer that createTextField turns into a
///        real TextField.
as_value
textfield_ctor(const fn_call& fn)
{

    as_object* obj = ensure<ValidThis>(fn);
    
    // It's not clear why this happens. Attaching a relay would have the
    // same effect as both following statements.
    obj->setArray(false);
    obj->setRelay(0);

    as_object* proto = obj->get_prototype();

    if (proto) {
        attachPrototypeProperties(*proto);
    }

    as_object* ar = getGlobal(fn).createArray();
    callMethod(ar, NSV::PROP_PUSH, obj);
    obj->set_member(NSV::PROP_uLISTENERS, ar);
    return as_value();
}


void
attachTextFieldInterface(as_object& o)
{
    // SWF6 or higher
    const int swf6Flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;

    VM& vm = getVM(o);
    o.init_member("replaceSel", vm.getNative(104, 100), swf6Flags);
    o.init_member("getTextFormat", vm.getNative(104, 101), swf6Flags);
    o.init_member("setTextFormat", vm.getNative(104, 102), swf6Flags);
    o.init_member("removeTextField", vm.getNative(104, 103), swf6Flags);
    o.init_member("getNewTextFormat", vm.getNative(104, 104), swf6Flags);
    o.init_member("setNewTextFormat",vm.getNative(104, 105), swf6Flags);
    o.init_member("getDepth", vm.getNative(104, 106), swf6Flags);

    // SWF7 or higher
    const int swf7Flags = as_object::DefaultFlags | PropFlags::onlySWF7Up;

    o.init_member("replaceText",vm.getNative(104, 107), swf7Flags);
    
    // TextField is an AsBroadcaster
    AsBroadcaster::initialize(o);

    // Finally ASSetPropFlags is called on the prototype.
    Global_as& gl = getGlobal(o);
    as_object* null = 0;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, &o, null, 131);
}

void
attachTextFieldStaticMembers(as_object& o)
{
    // SWF6 or higher
    const int swf6Flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
    VM& vm = getVM(o);
    o.init_member("getFontList", vm.getNative(104, 201), swf6Flags);
}


/// Return autoSize value as a string
//
/// @param val      AutoSize value 
/// @return         a C-string representation of the autoSize value.
///	                The return is *never* NULL.
const char*
autoSizeValueName(TextField::AutoSize val)
{
    switch (val) {
        case TextField::AUTOSIZE_LEFT:
            return "left";
        case TextField::AUTOSIZE_RIGHT:
            return "right";
        case TextField::AUTOSIZE_CENTER:
            return "center";
        case TextField::AUTOSIZE_NONE:
        default:
            return "none";
    }

}

TextField::AutoSize
parseAutoSize(const std::string& val)
{
    StringNoCaseEqual cmp;

    if (cmp(val, "left")) {
        return TextField::AUTOSIZE_LEFT;
    }
    if (cmp(val, "right")) {
        return TextField::AUTOSIZE_RIGHT;
    }
    if (cmp(val, "center")) {
        return TextField::AUTOSIZE_CENTER;
    }
    return TextField::AUTOSIZE_NONE;
}


} // anonymous namespace
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

