#ifndef MESSAGE_PUMP_INCLUDED
#define MESSAGE_PUMP_INCLUDED

class MessagePump
{
public:
	MessagePump();	

	bool update();
private:
	bool m_quitRequested;
};

#endif