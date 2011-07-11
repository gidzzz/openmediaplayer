/****************************************************************************
 **
 ** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #include <QtGui/qplaintextedit.h>
 #include <QtGui/qtextedit.h>
 #include <QtGui/qabstractkineticscroller.h>

 #ifndef TEXTEDITAUTORESIZER_H
 #define TEXTEDITAUTORESIZER_H

 class TextEditAutoResizer : public QObject
 {
     Q_OBJECT
 public:
     TextEditAutoResizer(QWidget *parent)
         : QObject(parent), plainTextEdit(qobject_cast<QPlainTextEdit *>(parent)),
           textEdit(qobject_cast<QTextEdit *>(parent)), edit(qobject_cast<QFrame *>(parent))
     {
         // parent must either inherit QPlainTextEdit or  QTextEdit!
         Q_ASSERT(plainTextEdit || textEdit);

         connect(parent, SIGNAL(textChanged()), this, SLOT(textEditChanged()));
         connect(parent, SIGNAL(cursorPositionChanged()), this, SLOT(textEditChanged()));
     }

 private Q_SLOTS:
     inline void textEditChanged();

 private:
     QPlainTextEdit *plainTextEdit;
     QTextEdit *textEdit;
     QFrame *edit;
 };

 void TextEditAutoResizer::textEditChanged()
 {
     QTextDocument *doc = textEdit ? textEdit->document() : plainTextEdit->document();
     QRect cursor = textEdit ? textEdit->cursorRect() : plainTextEdit->cursorRect();

     QSize s = doc->size().toSize();
     if (plainTextEdit)
         s.setHeight((s.height() + 1) * edit->fontMetrics().lineSpacing());

     const QRect fr = edit->frameRect();
     const QRect cr = edit->contentsRect();

     edit->setMinimumHeight(qMax(70, s.height() + (fr.height() - cr.height() - 1)));

     // make sure the cursor is visible in case we have a QAbstractScrollArea parent
     QPoint pos = edit->pos();
     QWidget *pw = edit->parentWidget();
     while (pw) {
         if (pw->parentWidget()) {
             if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(
                             pw->parentWidget()->parentWidget())) {
                 if (QAbstractKineticScroller * scroller=
                         area->property("kineticScroller").value<QAbstractKineticScroller *>()) {
                     scroller->ensureVisible(pos + cursor.center(), 10 + cursor.width(),
                                             2 * cursor.height());
                 }
                 break;
             }
         }
         pos = pw->mapToParent(pos);
         pw = pw->parentWidget();
     }
 }

 #endif
