#ifndef ENGINETEST_SCRIPTEDITORWIDGET_H_
#define ENGINETEST_SCRIPTEDITORWIDGET_H_

#include "ui_ScriptEditorWidget.h"

class ScriptEditorWidget : public QWidget, private Ui::ScriptEditorWidget
{
	Q_OBJECT

public:
	ScriptEditorWidget(QWidget *parent = 0);

	QString getScriptCode(void);

signals:
	void start(void);
	void stop(void);

};

#endif /*ENGINETEST_SCRIPTEDITORWIDGET_H_*/
