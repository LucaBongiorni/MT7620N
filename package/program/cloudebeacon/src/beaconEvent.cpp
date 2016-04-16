



void InitTimeEventList(TimeEventList* timeList)
{
	INIT_LIST_HEAD(&(timeList->head));
    pthread_mutex_init(&(timeList->EventMutex), NULL);
	return ;
}

void UninitTimeEventList(TimeEventList* timeList)
{
	DelAllEventNode(timeList);
    pthread_mutex_destroy(&timeList->EventMutex);
	return ;
}


TimeEventList::TimeEventList()
{
	INIT_LIST_HEAD(&m_head);
    pthread_mutex_init(&m_eventMutex, NULL);
}

TimeEventList::~TimeEventList()
{
	DelAllEventNode();
    pthread_mutex_destroy(&m_eventMutex);
}












