#include "ScriptEditorWidget.h"

#include <QFile>
#include <QTextStream>

namespace Thermite
{
	ScriptEditorWidget::ScriptEditorWidget(QWidget *parent)
	:QWidget(parent, Qt::Tool)
	{
		setupUi(this);

		connect(m_pStartButton, SIGNAL(clicked(void)), this, SIGNAL(start(void)));
		connect(m_pStopButton, SIGNAL(clicked(void)), this, SIGNAL(stop(void)));
	}

	void ScriptEditorWidget::setScriptCode(const QString& scriptCode)
	{
		return m_pTextEdit->setPlainText(scriptCode);
	}

	QString ScriptEditorWidget::getScriptCode(void)
	{
		return m_pTextEdit->toPlainText();
	}
}
