##cookbook_dbconnect.rb
require 'rubygems'
require 'dbi'
require 'active_record'
require 'og'

def with_db
	DBI.connect("dbi:Mysql:cookbook:localhost", "root", "111111") do |c|
		yield c
	end
end

def activerecord_connect
	ActiveRecord::Base.establish_connection(:adapter => "mysql", :host => "localhost", :username => "root", 
											:password => "111111", :database => "cookbook")
end

def og_connect
	Og.setup( { :destroy => false,
				:store => :mysql,
				:user => "root",
				:password => "111111",
				:name => "cookbook" } )
end

