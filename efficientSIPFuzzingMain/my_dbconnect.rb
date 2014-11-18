require './mydebug'
require 'rubygems'
require 'mysql'
#require './cookbook_dbconnect'

class Mysql 
	alias :query_no_block :query
	def query(sql)
		res = query_no_block(sql)
		return res unless block_given?
		begin
			yield res
		ensure
			res.free if res
		end
	end
end


class DbConnect
	include MyDebug

	def initialize(_database="", _user="root", _passwd="111111")
		super()
		@database = _database
		@user = _user
		@passwd = _passwd
	end
	def mysql(opts, stream)
		IO.popen("mysql #{opts}", "w") { |io| io.puts stream }
	end
	def with_db
		dbh = Mysql.real_connect("localhost", @user, @passwd, @database)
		begin
			yield dbh
		ensure	
			dbh.close
		end
	end
end
#test_db = DbConnect.new("cookbook")
=begin
	test_db.mysql '-uroot -p111111', <<-end
		drop database if exists website_db;
		create database website_db;
		grant all on website_db.* to #{`id -un`.strip}@localhost;
	end
=end

=begin
test_db.with_db do |db|
	db.query('drop table if exists secrets')
	db.query('create table secrets(id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, secret varchar(255))')
	db.query(%{insert into secrets(secret) values ("Oh, MySQL, you're the only one who really understands me.")})
end

begin
test_db.with_db do |db|
	res = db.query('select * from secrets')
	res.each{|row| puts "#{row[0]}: #{row[1]}"}
	res.free
end
end
test_db.with_db do |db|
	db.query('show tables;') do |res|
		_exist = 0
		res.each_hash do |hash|
			hash.each do |k, v| 
				puts "#{k} : #{v}"
				if v == "serets"
					_exist = 1
				end
			end
		end
		_exist
	end
end

=end