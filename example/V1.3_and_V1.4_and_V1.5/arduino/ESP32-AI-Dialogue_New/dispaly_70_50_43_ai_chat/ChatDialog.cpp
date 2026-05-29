#include "ChatDialog.h"
#include "img/elecrow_logo.h"
#include "img/ai_logo.h"
#include "img/user_logo.h"
#include "img/mic_logo.h"
#include "img/spk_logo.h"

ChatDialog::ChatDialog() {}

void ChatDialog::showMicOrSpk(int status) {
  currentMicSpkStatus = status;
  int x = tft.width() / 2 - 40;
  int y = 20;
  int w = 40;
  int h = 40;

  if (status == 1) {
    tft.pushImage(x, y, w, h, MIC_LOGO);
  } else if (status == 2) {
    tft.pushImage(x, y, w, h, SPK_LOGO);
  } else {
    tft.fillRect(x, y, w, h, TFT_WHITE);
  }
}





void ChatDialog::begin() {

  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(0);

  drawStaticElements();
}

void ChatDialog::addMessage(String text, bool isAI) {
  Serial.println("The received data:");
  Serial.println(text);

  if (messageCount >= MAX_MESSAGES) {
    for (int i = 0; i < MAX_MESSAGES - 1; i++) {
      messages[i] = messages[i + 1];
    }
    messageCount = MAX_MESSAGES - 1;
  }

  messages[messageCount].text = text;
  messages[messageCount].isAI = isAI;
  messages[messageCount].currentLength = isAI ? 0 : text.length();
  messageCount++;

  recalculateMessageLayout();
  scrollPosition = max(0, totalContentHeight - (DIALOG_HEIGHT - DIALOG_INNER_PADDING * 2));
  drawAllMessages();
}

void ChatDialog::appendToLastMessage(String text) {
  addMessage(text, true);
}

void ChatDialog::update() {
  if (millis() - lastCharUpdate > CHAR_UPDATE_INTERVAL) {
    lastCharUpdate = millis();

    for (int i = 0; i < messageCount; i++) {
      if (messages[i].isAI && messages[i].currentLength < messages[i].text.length()) {
        int nextEnd = findNextWordEnd(messages[i].text, messages[i].currentLength);
        if (nextEnd > messages[i].currentLength) {
          messages[i].currentLength = nextEnd;
        } else {
          messages[i].currentLength = messages[i].text.length();
        }
        drawSingleMessage(i);
      }
    }
  }
}

int ChatDialog::findNextWordEnd(const String &text, int startPos) {
  int len = text.length();
  int endPos = startPos;

  while (endPos < len && text[endPos] == ' ') endPos++;
  while (endPos < len && text[endPos] != ' ') endPos++;
  while (endPos < len && text[endPos] == ' ') endPos++;

  return endPos;
}

void ChatDialog::drawStaticElements() {
  tft.fillScreen(TFT_WHITE);
  tft.drawRoundRect(tft.width() / 2 - DIALOG_WIDTH / 2,
                    DIALOG_MARGIN_TOP,
                    DIALOG_WIDTH,
                    DIALOG_HEIGHT,
                    20,
                    TFT_DARKGREY);

  tft.setTextSize(3);
  tft.setTextColor(TFT_BLUE, TFT_WHITE);
  tft.setCursor(30, 30);
  tft.print("DEEPSEEK");
  tft.pushImage(tft.width() - 180, 20, 150, 31, ELECROW_LOGO);

  if (currentMicSpkStatus == 1) {
    tft.pushImage(tft.width() / 2 - 40, 20, 40, 40, MIC_LOGO);
  } else if (currentMicSpkStatus == 2) {
    tft.pushImage(tft.width() / 2 - 40, 20, 40, 40, SPK_LOGO);
  }
}

void ChatDialog::recalculateMessageLayout() {
  totalContentHeight = 0;
  int currentY = DIALOG_INNER_PADDING;

  for (int i = 0; i < messageCount; i++) {
    int textWidth = BUBBLE_CONTENT_WIDTH - TEXT_MARGIN * 2;
    int textHeight = calculateTextHeight(messages[i].text, textWidth);
    int bubbleHeight = textHeight + TEXT_MARGIN * 2;
    messages[i].height = max(bubbleHeight, AVATAR_SIZE);
    messages[i].y = currentY;
    currentY += messages[i].height + BUBBLE_MARGIN;
    totalContentHeight = currentY + DIALOG_INNER_PADDING;
  }
}

void ChatDialog::drawAllMessages() {
  drawStaticElements();

  tft.setClipRect(tft.width() / 2 - DIALOG_WIDTH / 2,
                  DIALOG_MARGIN_TOP + DIALOG_INNER_PADDING,
                  DIALOG_WIDTH,
                  DIALOG_HEIGHT - DIALOG_INNER_PADDING * 2);

  for (int i = 0; i < messageCount; i++) {
    int bubbleY = DIALOG_MARGIN_TOP + messages[i].y - scrollPosition;
    int messageBottom = bubbleY + messages[i].height;

    if (messageBottom > (DIALOG_MARGIN_TOP + DIALOG_INNER_PADDING) && bubbleY < (DIALOG_MARGIN_TOP + DIALOG_HEIGHT - DIALOG_INNER_PADDING)) {
      drawSingleMessage(i);
    }
  }
  tft.clearClipRect();
}

void ChatDialog::drawSingleMessage(int index) {
  Message msg = messages[index];
  int bubbleContentWidth = BUBBLE_CONTENT_WIDTH;
  int bubbleWidth = bubbleContentWidth + ARROW_WIDTH;
  int dialogX = tft.width() / 2 - DIALOG_WIDTH / 2;

  int avatarX = msg.isAI ? (dialogX + AVATAR_MARGIN)
                         : (dialogX + DIALOG_WIDTH - AVATAR_SIZE - AVATAR_MARGIN);
  int baseY = DIALOG_MARGIN_TOP + DIALOG_INNER_PADDING + msg.y - scrollPosition;

  int bubbleX, arrowX, arrowY;
  if (msg.isAI) {
    bubbleX = avatarX + AVATAR_SIZE + AVATAR_MARGIN - ARROW_WIDTH;
    arrowX = bubbleX + ARROW_WIDTH;
    arrowY = baseY + AVATAR_SIZE / 2 - ARROW_WIDTH / 2;
  } else {
    bubbleX = avatarX - bubbleWidth;
    arrowX = bubbleX + bubbleContentWidth;
    arrowY = baseY + AVATAR_SIZE / 2 - ARROW_WIDTH / 2;
  }

  tft.pushImage(avatarX, baseY, AVATAR_SIZE, AVATAR_SIZE, msg.isAI ? AI_LOGO : USER_LOGO);

  tft.fillRoundRect(
    msg.isAI ? (bubbleX + ARROW_WIDTH) : bubbleX,
    baseY,
    bubbleContentWidth,
    msg.height,
    BUBBLE_RADIUS,
    msg.isAI ? 0xE71C : 0xBDF7);

  if (msg.isAI) {
    tft.fillTriangle(
      bubbleX + ARROW_WIDTH, arrowY,
      bubbleX + ARROW_WIDTH, arrowY + ARROW_WIDTH,
      bubbleX, arrowY + ARROW_WIDTH / 2,
      0xE71C);
  } else {
    tft.fillTriangle(
      arrowX, arrowY,
      arrowX, arrowY + ARROW_WIDTH,
      arrowX + ARROW_WIDTH, arrowY + ARROW_WIDTH / 2,
      0xBDF7);
  }

  String displayText = msg.text.substring(0, msg.currentLength);
  drawWrappedText(
    displayText,
    msg.isAI ? (bubbleX + ARROW_WIDTH + TEXT_MARGIN) : (bubbleX + TEXT_MARGIN),
    baseY + TEXT_MARGIN,
    BUBBLE_CONTENT_WIDTH - TEXT_MARGIN * 2,
    msg.isAI ? TFT_BLACK : TFT_NAVY);
}

int ChatDialog::calculateTextHeight(String text, int maxWidth) {
  tft.setTextSize(TEXT_SIZE);
  int charWidth = CHAR_WIDTH * TEXT_SIZE;
  int spaceWidth = charWidth;
  int lines = 1;
  int currentLineWidth = 0;
  int currentWordWidth = 0;

  for (unsigned int i = 0; i < text.length(); i++) {
    char c = text[i];

    if (c == ' ' || c == '\n') {
      if (currentWordWidth > 0) {
        if (currentLineWidth + currentWordWidth > maxWidth) {
          lines++;
          currentLineWidth = currentWordWidth;
        } else {
          currentLineWidth += currentWordWidth;
        }
        currentWordWidth = 0;
      }

      if (c == '\n') {
        lines++;
        currentLineWidth = 0;
      } else if (currentLineWidth + spaceWidth > maxWidth) {
        lines++;
        currentLineWidth = spaceWidth;
      } else {
        currentLineWidth += spaceWidth;
      }
    } else {
      currentWordWidth += charWidth;
      if (currentWordWidth > maxWidth) {
        lines++;
        currentLineWidth = currentWordWidth - charWidth;
        currentWordWidth = charWidth;
      }
    }
  }

  if (currentWordWidth > 0) {
    if (currentLineWidth + currentWordWidth > maxWidth) lines++;
  }

  return lines * LINE_HEIGHT;
}

void ChatDialog::drawWrappedText(String text, int x, int y, int maxWidth, uint16_t color) {
  tft.setTextColor(color);
  tft.setTextSize(TEXT_SIZE);

  int charWidth = CHAR_WIDTH * TEXT_SIZE;
  int spaceWidth = charWidth;
  int cursorX = x;
  int cursorY = y;
  int currentLineWidth = 0;
  String currentWord = "";
  int currentWordWidth = 0;

  for (unsigned int i = 0; i < text.length(); i++) {
    char c = text[i];

    if (c == ' ' || c == '\n') {
      if (currentWord.length() > 0) {
        if (currentLineWidth + currentWordWidth > maxWidth) {
          cursorY += LINE_HEIGHT;
          cursorX = x;
          currentLineWidth = 0;
        }
        tft.setCursor(cursorX, cursorY);
        tft.print(currentWord);
        cursorX += currentWordWidth;
        currentLineWidth += currentWordWidth;
        currentWord = "";
        currentWordWidth = 0;
      }

      if (c == '\n') {
        cursorY += LINE_HEIGHT;
        cursorX = x;
        currentLineWidth = 0;
      } else {
        if (currentLineWidth + spaceWidth > maxWidth) {
          cursorY += LINE_HEIGHT;
          cursorX = x;
          currentLineWidth = 0;
        }
        tft.drawChar(' ', cursorX, cursorY);
        cursorX += spaceWidth;
        currentLineWidth += spaceWidth;
      }
    } else {
      currentWord += c;
      currentWordWidth += charWidth;

      if (currentWordWidth > maxWidth) {
        int availableWidth = maxWidth - currentLineWidth;
        int charsToDraw = availableWidth / charWidth;

        if (charsToDraw > 0) {
          tft.setCursor(cursorX, cursorY);
          tft.print(currentWord.substring(0, charsToDraw));
          cursorX += charsToDraw * charWidth;
          currentLineWidth += charsToDraw * charWidth;
        }

        cursorY += LINE_HEIGHT;
        cursorX = x;
        currentWord = currentWord.substring(charsToDraw);
        currentWordWidth = currentWord.length() * charWidth;
        currentLineWidth = 0;
      }
    }
  }

  if (currentWord.length() > 0) {
    if (currentLineWidth + currentWordWidth > maxWidth) {
      cursorY += LINE_HEIGHT;
      cursorX = x;
    }
    tft.setCursor(cursorX, cursorY);
    tft.print(currentWord);
  }
}