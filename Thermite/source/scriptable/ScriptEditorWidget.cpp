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

		QFile updateScriptFile("..\\share\\thermite\\apps\\TechDemo\\scripts\\update.js");

		if (updateScriptFile.open(QFile::ReadOnly))
		{
			QTextStream stream(&updateScriptFile);
			m_pTextEdit->setPlainText(stream.readAll());
			updateScriptFile.close();
		}
		else
		{
			m_pTextEdit->setPlainText("//Failed to open file");
		}
	}

	QString ScriptEditorWidget::getScriptCode(void)
	{
		return m_pTextEdit->toPlainText();
	}
}
