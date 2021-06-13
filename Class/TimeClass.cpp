#include"TimeClass.h"

using namespace std;


TimeClass::TimeClass(time_t time_in)
	:t(time_in)
{
#if defined __linux__
	localtime_r(&(this->t), &(this->t_tm));
#elif defined _WIN32||_WIN64
	localtime_s(&(this->t_tm), &(this->t));
#endif // _WIN
}

std::string TimeClass::TimeNow()
{
	string re="";
	re += to_string(1900 + this->t_tm.tm_year);
	re += "/" + to_string(this->t_tm.tm_mon + 1);
	re += "/" + to_string(this->t_tm.tm_mday);
	re += "-" + to_string(this->t_tm.tm_hour);
	re += ":" + to_string(this->t_tm.tm_min);
	re += ":" + to_string(this->t_tm.tm_sec);
	return re;
}

bool TimeClass::init() {
	try
	{
		time(&(this->t));
#if defined __linux__
		localtime_r(&(this->t), &(this->t_tm));
#elif defined _WIN32||_WIN64
		localtime_s(&(this->t_tm), &(this->t));
#endif // _WIN
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

std::string TimeClass::timeFromTm(DateFormat dateformat) {
	std::string str = "";
	switch (dateformat)
	{
	case TimeClass::DateFormat::SEC:
		str = std::to_string(t_tm.tm_sec) + "��";
		break;
	case TimeClass::DateFormat::MIN:
		str = std::to_string(t_tm.tm_min) + "��";
		break;
	case TimeClass::DateFormat::HOUR:
		str = std::to_string(t_tm.tm_hour) + "ʱ";
		break;
	case TimeClass::DateFormat::DAY:
		str = std::to_string(t_tm.tm_mday) + "��";
		break;
	case TimeClass::DateFormat::MON:
		str = std::to_string(t_tm.tm_mon + 1) + "��";
		break;
	case TimeClass::DateFormat::YEAR:
		str = std::to_string(t_tm.tm_year + 1900) + "��";
		break;
	case TimeClass::DateFormat::WEEKDAY:
		switch (t_tm.tm_wday)
		{
		case 0:
			str = "����һ";
		case 1:
			str = "���ڶ�";
		case 2:
			str = "������";
		case 3:
			str = "������";
		case 4:
			str = "������";
		case 5:
			str = "������";
		case 6:
			str = "������";
		default:
			break;
		}
		break;
	case TimeClass::DateFormat::YEARDAY:
		str = "����ĵ�" + std::to_string(t_tm.tm_yday + 1) + "��";
		break;
	default:
		break;
	}
	return str;
}