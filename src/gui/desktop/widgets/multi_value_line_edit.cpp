/*
 * Multi value line edit - for multi completition
 * Copyright (C) 2016  Michał Walenciak <Kicer86@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "multi_value_line_edit.hpp"

#include <cassert>

#include <QCompleter>


MultiValueLineEdit::MultiValueLineEdit(QWidget* parent):
    QLineEdit(parent),
    m_completer(nullptr)
{

}


MultiValueLineEdit::MultiValueLineEdit(const QString& text, QWidget* parent): QLineEdit(text, parent)
{

}


void MultiValueLineEdit::setCompleter(QCompleter* completer)
{
    assert(m_completer == nullptr);

    completer->setWidget(this);
    connect(completer, static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated),
            this, &MultiValueLineEdit::insertCompletion);

    m_completer = completer;
}



void MultiValueLineEdit::keyPressEvent(QKeyEvent* e)
{
    QLineEdit::keyPressEvent(e);

    const QString v = text();
    const std::pair<int, int> wordPos = currentWordPos();
    const QString word = v.mid(wordPos.first, wordPos.second);
    const QString wordToComplete = word.trimmed();

    m_completer->setCompletionPrefix(wordToComplete);
    m_completer->complete();
}


std::pair<int, int> MultiValueLineEdit::currentWordPos() const
{
    const int cp = cursorPosition();
    const QString v = text();
    const QString l = v.left(cp);
    const int wb = l.lastIndexOf(",") + 1;

    return std::make_pair(wb, cp);
}


void MultiValueLineEdit::insertCompletion(const QString& completion)
{

}
