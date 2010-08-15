#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <QHash>

class Keyboard : public QObject
	{
		Q_OBJECT

	public slots:
		bool isPressed(int key);
		void press(int key);
		void release(int key);

	private:
		QHash<int, bool> mKeyStates;
	};	

#endif //KEYBOARD_H_