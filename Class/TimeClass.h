#pragma once
#include<string>
#include<cstring>
#include<cstdarg>
#include<ctime>
#include<cstdlib>
#include<memory>

class TimeClass {
public:
	enum class DateFormat
	{
		SEC,
		MIN,
		HOUR,
		DAY,
		MON,
		YEAR,
		WEEKDAY,
		YEARDAY,
	};
public:
	TimeClass(time_t);

	TimeClass() { if (!(this->init())) { this->t = 0; memset(&(this->t_tm), 0, sizeof(this->t_tm)); }; }
	/**
	*����������ĸ�ʽ���ָ����ʱ���ַ���
	*/
	template<typename ...Arg>
	std::string Time_Now_str(DateFormat dataFormat, Arg...arg) {
		return this->getTime(dataFormat, arg...);
	}

	/**
	*����ʱ���ַ�������ʽΪxxxx/xx/xx-xx:xx:xx
	*/
	std::string TimeNow();
	/**
	*���ص�ǰʱ���tm�ṹ��
	*/
	tm Time_Now_tm() { return this->t_tm; }

	/**
	���ص�ǰʱ���time_tֵ����1900������ڵ�������
	*/
	time_t Time_Now_time_t() { return this->t; }

private:
	bool init();

	template<typename ...Arg>
	std::string getTime(TimeClass::DateFormat dateformat, Arg...arg) {
		return this->timeFromTm(dateformat) + this->getTime(arg...);
	}

	std::string getTime() {
		return "";
	}

	std::string timeFromTm(DateFormat dateformat);
private:
	time_t t;
	tm t_tm;
};