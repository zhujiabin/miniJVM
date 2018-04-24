/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.gui;

import java.io.UnsupportedEncodingException;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.mini.glfm.Glfm;
import static org.mini.nanovg.Gutil.toUtf8;
import static org.mini.gui.GToolkit.nvgRGBA;
import org.mini.nanovg.Nanovg;
import static org.mini.nanovg.Nanovg.NVG_ALIGN_LEFT;
import static org.mini.nanovg.Nanovg.NVG_ALIGN_TOP;
import static org.mini.nanovg.Nanovg.nvgBeginPath;
import static org.mini.nanovg.Nanovg.nvgBoxGradient;
import static org.mini.nanovg.Nanovg.nvgCreateNVGglyphPosition;
import static org.mini.nanovg.Nanovg.nvgCreateNVGtextRow;
import static org.mini.nanovg.Nanovg.nvgFill;
import static org.mini.nanovg.Nanovg.nvgFillColor;
import static org.mini.nanovg.Nanovg.nvgFillPaint;
import static org.mini.nanovg.Nanovg.nvgFontFace;
import static org.mini.nanovg.Nanovg.nvgFontSize;
import static org.mini.nanovg.Nanovg.nvgNVGglyphPosition_x;
import static org.mini.nanovg.Nanovg.nvgRestore;
import static org.mini.nanovg.Nanovg.nvgRoundedRect;
import static org.mini.nanovg.Nanovg.nvgSave;
import static org.mini.nanovg.Nanovg.nvgStroke;
import static org.mini.nanovg.Nanovg.nvgStrokeColor;
import static org.mini.nanovg.Nanovg.nvgTextAlign;
import static org.mini.nanovg.Nanovg.nvgTextBreakLinesJni;
import static org.mini.nanovg.Nanovg.nvgTextGlyphPositionsJni;
import static org.mini.nanovg.Nanovg.nvgTextJni;
import static org.mini.nanovg.Nanovg.nvgTextMetrics;

/**
 *
 * @author gust
 */
public class GTextBox extends GObject {

    String hint;
    byte[] hint_arr;
    StringBuilder textsb = new StringBuilder();
    byte[] text_arr;
    char preicon;
    boolean singleMode;//single line mode
    //
    float[] lineh = {0};
    private int caretIndex;//光标在字符串中的位置

    boolean drag = false;
    int selectStart = -1;//选取开始
    int selectEnd = -1;//选取结束
    int totalRows;//字符串总行数，动态计算出
    int showRows;//可显示行数
    int topShowRow;//显示区域第一行的行号
    short[][] area_detail;
    int scrollDelta;
    //
    static final int AREA_DETAIL_ADD = 7;//额外增加slot数量
    static final int AREA_START = 4;//字符串起点位置
    static final int AREA_END = 5;//字符终点位置
    static final int AREA_ROW = 6;//行号
    static final int AREA_X = LEFT;
    static final int AREA_Y = TOP;
    static final int AREA_W = WIDTH;
    static final int AREA_H = HEIGHT;

    public GTextBox(String text, String hint, int left, int top, int width, int height) {
        setText(text);
        setHint(hint);
        boundle[LEFT] = left;
        boundle[TOP] = top;
        boundle[WIDTH] = width;
        boundle[HEIGHT] = height;
    }

    public void setHint(String hint) {
        this.hint = hint;
        hint_arr = toUtf8(hint);
    }

    public void setText(String text) {
        this.textsb.setLength(0);
        this.textsb.append(text);
    }

    public String getText() {
        return textsb.toString();
    }

    public void setSingleMode(boolean single) {
        this.singleMode = single;
    }

    boolean isInArea(short[] bound, float x, float y) {
        return x >= bound[LEFT] && x <= bound[LEFT] + bound[WIDTH]
                && y >= bound[TOP] && y <= bound[TOP] + bound[HEIGHT];
    }

    /**
     * 返回指定位置所在字符串中的位置
     *
     * @param x
     * @param y
     * @return
     */
    int getCaretIndexFromArea(int x, int y) {
        if (area_detail != null) {
            for (short[] detail : area_detail) {
                if (detail != null) {
                    if (isInArea(detail, x, y)) {
                        for (int i = AREA_DETAIL_ADD, imax = detail.length; i < imax; i++) {
                            int x0 = detail[i];
                            int x1 = (i + 1 < imax) ? detail[i + 1] : detail[AREA_X] + detail[AREA_W];
                            if (x >= x0 && x < x1) {
                                if (x > detail[detail.length - 1]) {
                                    return detail[AREA_START] + (i - AREA_DETAIL_ADD) + 1;
                                } else {
                                    return detail[AREA_START] + (i - AREA_DETAIL_ADD);
                                }
                            }
                        }
                        return detail[AREA_END];

                    }
                }
            }
        }
        return -1;
    }

    /**
     * 返回光标当前所在的x,y坐标,及行号,数组下标
     *
     * @return
     */
    int[] getCaretPosFromArea() {
        if (area_detail != null) {
            int i = 0;
            for (short[] detail : area_detail) {
                if (detail != null) {
                    if (caretIndex >= detail[AREA_START] && caretIndex < detail[AREA_END]) {
                        int idx = caretIndex - detail[AREA_START] + AREA_DETAIL_ADD - 1;
                        int x = idx < detail[AREA_END] ? detail[idx] : detail[AREA_X] + +detail[AREA_W];
                        return new int[]{x + (int) lineh[0] / 2, detail[AREA_Y] + (int) lineh[0] / 2, detail[AREA_ROW], i};
                    }
                }
                i++;
            }
        }
        return null;
    }

    @Override
    public void touchEvent(int phase, int x, int y) {
        int rx = (int) (x - parent.getX());
        int ry = (int) (y - parent.getY());
        if (isInBoundle(boundle, rx, ry)) {
            if (phase == Glfm.GLFMTouchPhaseBegan) {
                parent.setFocus(this);
                int caret = getCaretIndexFromArea(x, y);
                if (caret >= 0) {
                    setCaretIndex(caret);
                    resetSelect();
                    selectStart = caret;
                    //drag = true;//打开可以进行选择模式，但手机上滚动和选取不可同时使用
                }
            } else if (phase == Glfm.GLFMTouchPhaseMoved) {
                if (drag) {
                    int caret = getCaretIndexFromArea(x, y);
                    if (caret >= 0) {
                        selectEnd = caret;
                    }
                }
            } else if (actionListener != null) {
                actionListener.action();
            }
        }
        if (phase == Glfm.GLFMTouchPhaseEnded) {
            drag = false;
            if (selectEnd == -1 || selectStart == selectEnd) {
                resetSelect();
            }
        }

    }

    @Override
    public void clickEvent(int x, int y) {
        int rx = (int) (x - parent.getX());
        int ry = (int) (y - parent.getY());
        if (isInBoundle(boundle, rx, ry)) {
            int caret = getCaretIndexFromArea(x, y);
            if (caret >= 0) {
                int[] pos = getCaretPosFromArea();
                //找光标附近的单词
                if (pos != null) {
                    int arrIndex = pos[3];
                    short[] arr = area_detail[arrIndex];
                    String s = textsb.substring(arr[AREA_START], arr[AREA_END]);
                    int strIndex = caret - arr[AREA_START];
                    int after = s.indexOf(' ', strIndex);
                    if (after >= 0) {
                        int before = s.substring(0, strIndex).lastIndexOf(' ') + 1;
                        selectStart = before < 0 ? arr[AREA_START] : arr[AREA_START] + before;
                        selectEnd = after < 0 ? arr[AREA_END] : arr[AREA_START] + after;
                    }
                }
            }
        }
    }

    @Override
    public void onFocus() {
        Glfm.glfmSetKeyboardVisible(getForm().getWinContext(), true);
    }

    @Override
    public void onUnFocus() {
        Glfm.glfmSetKeyboardVisible(getForm().getWinContext(), false);
    }

    /**
     *
     * @param str
     * @param mods
     * @param character
     */
    @Override
    public void characterEvent(String str, int mods) {
        if (parent.getFocus() != this) {
            return;
        }
        int[] selectFromTo = getSelected();
        if (selectFromTo != null) {
            delectSelect();
        }
        for (int i = 0, imax = str.length(); i < imax; i++) {
            char character = str.charAt(i);
            if (character != '\n' && character != '\r' || !singleMode) {
                textsb.insert(caretIndex, character);
                setCaretIndex(caretIndex + 1);
            }
        }
        text_arr = null;
    }

    @Override
    public void keyEvent(int key, int action, int mods) {
        if (parent.getFocus() != this) {
            return;
        }
        if (action == Glfm.GLFMKeyActionPressed || action == Glfm.GLFMKeyActionRepeated) {
            switch (key) {
                case Glfm.GLFMKeyBackspace: {
                    if (textsb.length() > 0 && caretIndex > 0) {
                        int[] selectFromTo = getSelected();
                        if (selectFromTo != null) {
                            delectSelect();
                        } else {
                            textsb.delete(caretIndex - 1, caretIndex);
                            setCaretIndex(caretIndex - 1);
                            text_arr = null;
                        }
                    }
                    break;
                }
//                case Glfm.GLFMKeyDelete: {
//                    if (textsb.length() > caretIndex) {
//                        int[] selectFromTo = getSelected();
//                        if (selectFromTo != null) {
//                            delectSelect();
//                        } else {
//                            textsb.delete(caretIndex, caretIndex + 1);
//                            text_arr = null;
//                        }
//                    }
//                    break;
//                }
                case Glfm.GLFMKeyEnter: {
                    String txt = getText();
                    if (txt != null && txt.length() > 0 && !singleMode) {
                        int[] selectFromTo = getSelected();
                        if (selectFromTo != null) {
                            delectSelect();
                        }
                        setCaretIndex(caretIndex + 1);
                        textsb.insert(caretIndex, "\n");
                        text_arr = null;
                    }
                    break;
                }
                case Glfm.GLFMKeyLeft: {
                    if (textsb.length() > 0 && caretIndex > 0) {
                        setCaretIndex(caretIndex - 1);
                    }
                    break;
                }
                case Glfm.GLFMKeyRight: {
                    if (textsb.length() > caretIndex) {
                        setCaretIndex(caretIndex + 1);
                    }
                    break;
                }
                case Glfm.GLFMKeyUp: {
                    int[] pos = getCaretPosFromArea();
                    if (topShowRow > 0 && (pos == null || pos[2] == topShowRow)) {
                        topShowRow--;
                    }
                    if (pos != null) {
                        int cart = getCaretIndexFromArea(pos[0], pos[1] - (int) lineh[0]);
                        if (cart >= 0) {
                            setCaretIndex(cart);
                        }
                    }
                    break;
                }
                case Glfm.GLFMKeyDown: {
                    int[] pos = getCaretPosFromArea();
                    if (topShowRow < totalRows - showRows && (pos == null || pos[2] == topShowRow + showRows - 1)) {
                        topShowRow++;
                    }
                    if (pos != null) {
                        int cart = getCaretIndexFromArea(pos[0], pos[1] + (int) lineh[0]);
                        if (cart >= 0) {
                            setCaretIndex(cart);
                        }

                    }
                    break;
                }
            }
        }
    }

    @Override
    public void scrollEvent(double scrollX, double scrollY, int x, int y) {
        int rx = (int) (x - parent.getX());
        int ry = (int) (y - parent.getY());
        if (isInBoundle(boundle, rx, ry)) {
            scrollDelta += scrollY;
            if (Math.abs(scrollDelta) > lineh[0]) {
                topShowRow -= scrollDelta / lineh[0];
                scrollDelta %= lineh[0];
            }
        }
    }

    void delectSelect() {
        int[] sarr = getSelected();
        setCaretIndex(sarr[0]);
        textsb.delete(sarr[0], sarr[1]);
        text_arr = null;
        resetSelect();
    }

    void resetSelect() {
        selectStart = selectEnd = -1;
    }

    int[] getSelected() {
        int select1 = 0, select2 = 0;
        if (selectStart != -1 && selectEnd != -1) {
            select1 = selectStart > selectEnd ? selectEnd : selectStart;
            select2 = selectStart < selectEnd ? selectEnd : selectStart;
            return new int[]{select1, select2};
        }
        return null;
    }

    /**
     *
     * @param vg
     * @return
     */
    @Override
    public boolean update(long vg) {
        float x = getX();
        float y = getY();
        float w = getW();
        float h = getH();

        drawTextBox(vg, x, y, w, h);
        return true;
    }

    void drawTextBox(long vg, float x, float y, float w, float h) {
        drawTextBoxBase(vg, x, y, w, h);
        nvgFontSize(vg, GToolkit.getStyle().getTextFontSize());
        nvgFontFace(vg, GToolkit.getFontWord());
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

        //字高
        nvgTextMetrics(vg, null, null, lineh);
        float lineH = lineh[0];

        float[] text_area = new float[]{x + 5f, y + 5f, w - 10f, h - 10f};
        float dx = text_area[LEFT];
        float dy = text_area[TOP];

        if (singleMode) {
            dy += .5f * (text_area[HEIGHT] - lineH);
        }
        //画文本或提示
        if ((getText() == null || getText().length() <= 0) && parent.getFocus() != this) {
            nvgFillColor(vg, GToolkit.getStyle().getHintFontColor());
            nvgTextJni(vg, dx, dy, hint_arr, 0, hint_arr.length);
        } else {//编辑中
            if (text_arr == null) {//文字被修改过
                text_arr = toUtf8(textsb.toString());
                showRows = Math.round(text_area[HEIGHT] / lineH);

                //用于存放屏墓中各行的一些位置信息
                area_detail = new short[showRows][];
                float[] bond = new float[4];
                Nanovg.nvgTextBoxBoundsJni(vg, dx, dy, text_area[WIDTH], text_arr, 0, text_arr.length, bond);
                totalRows = Math.round((bond[HEIGHT] - bond[TOP]) / lineH);
            }
            int posCount = 100;
            int rowCount = 10;
            long rowsHandle = nvgCreateNVGtextRow(rowCount);
            long glyphsHandle = nvgCreateNVGglyphPosition(posCount);
            int nrows, i, jchar_count, area_row_index = 0;
            float caretx = 0;

            nvgSave(vg);
            Nanovg.nvgScissor(vg, text_area[LEFT], text_area[TOP], text_area[WIDTH], text_area[HEIGHT]);
            //编辑区中最顶端文本的行号
            if (topShowRow < 0) {
                topShowRow = 0;
            } else if (topShowRow > totalRows - showRows) {
                topShowRow = totalRows - showRows;
            }

            //取选取的起始和终止位置
            int[] selectFromTo = getSelected();

            // The text break API can be used to fill a large buffer of rows,
            // or to iterate over the text just few lines (or just one) at a time.
            // The "next" variable of the last returned item tells where to continue.
            //取UTF8字符串的内存地址，供NATIVE API调用
            long ptr = GToolkit.getArrayDataPtr(text_arr);
            int start = 0;
            int end = text_arr.length;

            int char_at = 0;
            int char_starti, char_endi;

            //通过nvgTextBreakLinesJni进行断行
            while ((nrows = nvgTextBreakLinesJni(vg, text_arr, start, end, text_area[WIDTH], rowsHandle, rowCount)) != 0) {

                //循环绘制行
                for (i = 0; i < nrows; i++) {
                    if (area_row_index >= topShowRow && area_row_index < topShowRow + showRows) {
                        //取得第i 行的行宽
                        float row_width = Nanovg.nvgNVGtextRow_width(rowsHandle, i);

                        //返回 i 行的起始和结束位置
                        int byte_starti = (int) (Nanovg.nvgNVGtextRow_start(rowsHandle, i) - ptr);
                        int byte_endi = (int) (Nanovg.nvgNVGtextRow_end(rowsHandle, i) - ptr);
                        try {
                            String preStrs = new String(text_arr, 0, byte_starti, "utf-8");
                            char_at = preStrs.length();
                        } catch (UnsupportedEncodingException ex) {
                            ex.printStackTrace();
                        }
                        //从字节转成字符
                        String curRowStrs = "";
                        try {
                            curRowStrs = new String(text_arr, byte_starti, byte_endi - byte_starti, "utf-8");
                        } catch (UnsupportedEncodingException ex) {
                            ex.printStackTrace();
                        }
                        char_starti = char_at;
                        char_endi = char_at + curRowStrs.length() - 1;

                        System.out.println(char_starti + "\t" + char_endi + "\t" + byte_starti + "\t" + byte_endi + "\t\"" + curRowStrs + "\"");

                        caretx = dx;
                        //取得i行的各个字符的具体位置，结果存入glyphs
                        jchar_count = nvgTextGlyphPositionsJni(vg, dx, dy, text_arr, byte_starti, byte_endi, glyphsHandle, posCount);
                        int curRow = area_row_index - topShowRow;

                        //把这些信息存下来，用于在点击的时候找到点击了文本的哪个位置
                        //前面存固定信息
                        area_detail[curRow] = new short[AREA_DETAIL_ADD + jchar_count];
                        area_detail[curRow][AREA_X] = (short) dx;
                        area_detail[curRow][AREA_Y] = (short) dy;
                        area_detail[curRow][AREA_W] = (short) text_area[WIDTH];
                        area_detail[curRow][AREA_H] = (short) lineH;
                        area_detail[curRow][AREA_START] = (short) char_starti;
                        area_detail[curRow][AREA_END] = (short) char_endi;
                        area_detail[curRow][AREA_ROW] = (short) area_row_index;
                        //后面把每个char的位置存下来
                        for (int j = 0; j < jchar_count; j++) {
                            //取第 j 个字符的X座标
                            float x0 = nvgNVGglyphPosition_x(glyphsHandle, j);
                            area_detail[curRow][AREA_DETAIL_ADD + j] = (short) x0;
                        }

                        //计算下一行开始
                        char_at = char_at + curRowStrs.length();

                        if (parent.getFocus() == this) {
                            boolean draw = false;
                            if (caretIndex >= char_starti && caretIndex <= char_endi) {
                                caretx = area_detail[curRow][AREA_DETAIL_ADD + (caretIndex - char_starti)];
                                draw = true;
                                if (caretIndex != 0 && caretIndex - char_starti == 0) {//光标移到行首时，只显示在上一行行尾
                                    draw = false;
                                }
                            } else if (caretIndex == char_endi + 1) {
                                caretx = dx + row_width;
                                draw = true;
                            } else if (jchar_count == 0) {
                                caretx = dx;
                                draw = true;
                            }
                            if (draw) {
                                GToolkit.drawCaret(vg, caretx, dy, 1, lineH, false);
                            }
                        }

                        if (selectFromTo != null) {
                            float drawSelX = dx, drawSelW = row_width;
                            if (selectFromTo[0] < byte_endi && selectFromTo[0] > byte_starti) {
                                int pos = selectFromTo[0] - area_detail[curRow][AREA_START];
                                drawSelX = nvgNVGglyphPosition_x(glyphsHandle, pos);
                            }
                            if (selectFromTo[1] > byte_starti && selectFromTo[1] < byte_endi) {
                                int pos = selectFromTo[1] - area_detail[curRow][AREA_START];
                                drawSelW = nvgNVGglyphPosition_x(glyphsHandle, pos) - drawSelX;
                            }
                            if (byte_endi < selectFromTo[0] || byte_starti > selectFromTo[1]) {

                            } else {
                                GToolkit.drawSelect(vg, drawSelX, dy, drawSelW, lineH);
                            }
                        }
                        nvgFillColor(vg, GToolkit.getStyle().getTextFontColor());
                        nvgTextJni(vg, dx, dy, text_arr, byte_starti, byte_endi);

                        dy += lineH;
                    }
                    area_row_index++;
                }

                long next = Nanovg.nvgNVGtextRow_next(rowsHandle, nrows - 1);
                start = (int) (next - ptr);
            }
            Nanovg.nvgResetScissor(vg);
            nvgRestore(vg);

            Nanovg.nvgDeleteNVGtextRow(rowsHandle);
            Nanovg.nvgDeleteNVGglyphPosition(glyphsHandle);

        }
    }

    public static void drawTextBoxBase(long vg, float x, float y, float w, float h) {
        byte[] bg;
        // Edit
        bg = nvgBoxGradient(vg, x + 1, y + 1 + 1.5f, w - 2, h - 2, 3, 4, GToolkit.getStyle().getEditBackground(), nvgRGBA(32, 32, 32, 32));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x + 1, y + 1, w - 2, h - 2, 4 - 1);
        nvgFillPaint(vg, bg);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1, h - 1, 4 - 0.5f);
        nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 48));
        nvgStroke(vg);
    }

    /**
     * @param caretIndex the caretIndex to set
     */
    private void setCaretIndex(int caretIndex) {
        this.caretIndex = caretIndex;
    }

}
