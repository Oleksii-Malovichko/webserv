#pragma once

#include <string>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include "ClientSocket.hpp"


/* 
= delete - копирование запрещено
noexcept - означает что ф-я не выбрасывает исключение
Client(Client &&other) noexcept; - позволяет перенсти ресурсы из одного объекта в другой
Client& operator=(Client &&other) noexcept; - то же самое, но при присваивании уже существующего объекта
После move старый объект должен остаться в безопасном "пустом" состоянии

короче, вся эта дрочь нужна, чтобы нельзя было скопировать конструкторы, так как fd у каждого должен быть уникальный, а если есть больше одного объекта
с одинаковым fd, то у нас будет double-close


*/

class Client
{
	public:
		enum struct State
		{
			READING,
			WRITING,
			CLOSED
		};
	private:
		ClientSocket socket;
		std::string readBuffer;
		std::string writeBuffer;
		State state;
		std::chrono::steady_clock::time_point lastActivity;

	public:
		// контсруктор принимает уже созданный сокет (explicit нужен чтобы не было неявных преоброзований)
		explicit Client(ClientSocket &&sock);
		// перемещение, копирование запрещено
		Client(const Client&) = delete;
		Client& operator=(const Client&) = delete;
		// разрешить перемещение
		Client(Client &&other) noexcept;
		Client& operator=(Client &&other) noexcept;

		ssize_t readFromSocket(); // чтение данных в readBuffer
		ssize_t writeToSocket(); // запись данных в writeBuffer
		bool hasPendingWrite() const; // есть ли данные для записи
		void appendToWriteBuffer(const std::string &data); // добавить данные для отправки
		void updateLastActivity(); // обновление времени активности
		std::chrono::steady_clock::time_point getLastActivity() const;

		// эти ф-ии нужны для epoll!
		void close(); // закрытие соединения
		int getFD() const; // получить fd для epoll
		State getState() const;
		void setState(State newState);
		const std::string &getReadBuffer() const;
		void clearReadBuffer();
};
