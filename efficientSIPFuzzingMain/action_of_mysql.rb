##action_of_mysql.rb

module ActionOfMysql
	require './my_dbconnect'
=begin
	def write_to_mysql(params)
		sip_stack_name = params[:sip_stack_name]
		file_name = params[:file_name]
		function_list = params[:function_list]
		puts "sip_stack_name: #{sip_stack_name}, file_name:#{file_name}, function_list:#{function_list}"
		tquery = DbConnect.new("#{$SIPDB}")
		tquery.with_db do |db|
			res = db.query('select * from secrets')
			res.each{|row| puts "#{row[0]}: #{row[1]}"}
			res.free
		end
	end
=end
	def test_table_in_mysql(_table_name)
		tquery = DbConnect.new("#{$SIPDB}")
		tquery.with_db do |db|
			db.query('show tables;') do |res|
				_exist = 0
				res.each_hash do |hash|
					hash.each do |k, v| 
						#puts "#{k} : #{v} : #{_table_name}"
						if v == "#{_table_name}"
							_exist = 1
						end
					end
				end
				return _exist
			end
		end
	end
	def test_file_in_mysql(_files_table="", _file_name="")
		if _files_table == "" || _file_name == ""
			raise ArgumentError, "#{__FILE__}, #{__LINE__}, params error"
		end
		tquery = DbConnect.new("#{$SIPDB}")
		tquery.with_db do |db|
			db.query("select * from #{_files_table};") do |res|
				_file_exist = 0
				res.each_hash do |hash|
					hash.each do |k, v|
						#puts "#{k} : #{v}"
						if v == _file_name
							_file_exist = 1
						end
					end
				end
				_file_exist
			end
		end
	end
	def create_table_mysql(table_name, params = "")
		tquery = DbConnect.new("#{$SIPDB}")
		debug_puts "#{$SIPDB}"
		if table_name =~ /files_table/
			params = "(id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, 
													file_name varchar(255),
													function_list longtext,
													file_headers longtext,
													file_base_path varchar(255),
													sip_header_from_file longtext,
													sip_header_from_file_content longtext)"
			
		elsif table_name =~ /funcs_table/
			params = "(id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, 
													file_id INT,
													file_name varchar(255),
													function_name varchar(255),
													sub_function_list longtext,
													sip_header_from_function longtext)"
		else
			raise StandardError, "#{__FILE__}:#{__LINE__}:create_table error"
		end
		tquery.with_db do |db|
			db.query("create table #{table_name}#{params};") do |res|
			end
		end
	end
	def drop_table_mysql(files_table)
		tquery = DbConnect.new("#{$SIPDB}")
		tquery.with_db do |db|
			db.query("drop table if exists #{files_table};") do |res|
			end
		end
	end
	def insert_file_to_mysql(params)
		sip_stack_name = params[:sip_stack_name]
		file_name = params[:file_name]
		function_list = params[:function_list]
		drop_table = params[:drop_table]
		file_headers = params[:file_headers]
		file_base_path = params[:file_base_path]
		sip_header_from_file = params[:sip_header_from_file]
		sip_header_from_file_content = params[:sip_header_from_file_content]

		files_table = "#{sip_stack_name}_files_table"
		tquery = DbConnect.new("#{$SIPDB}")
		#debug_puts "insert into #{files_table}(file_name) values (\"#{file_name}\", \"#{function_list}\");"
		tquery.with_db do |db|
			db.query("insert into #{files_table}(file_name, function_list, file_headers, file_base_path, sip_header_from_file, sip_header_from_file_content) 
					values (\'#{file_name}\', \'#{function_list}\', \'#{file_headers}\', \'#{file_base_path}\', \'#{sip_header_from_file}\', \'#{sip_header_from_file_content}\');") do |res|
			end
		end
	end
	def write_file_to_mysql(params)
		sip_stack_name = params[:sip_stack_name]
		file_name = params[:file_name]
		function_list = params[:function_list]
		drop_table = params[:drop_table]
		if sip_stack_name == "" || file_name == ""
			raise ArgumentError, "#{__FILE__}, #{__LINE__}, params error"
		end
		_table_exist = 0
		files_table = "#{sip_stack_name}_files_table"
		funcs_table = "#{sip_stack_name}_funcs_table"
		#puts files_table
		if drop_table == 1
			drop_table_mysql(files_table)
			create_table_mysql(files_table)
			drop_table_mysql(funcs_table)
			create_table_mysql(funcs_table)
		end
		_table_exist = test_table_in_mysql(files_table)
		#p _table_exist
		if _table_exist == 1
			_file_exist = test_file_in_mysql(files_table, file_name)
			if _file_exist == 1
				#raise StandardError, "#{__FILE__}, #{__LINE__}, params error"
				log()
			else
				insert_file_to_mysql(params)
			end
		else
			raise StandardError, "#{__FILE__}:#{__LINE__}:create_table error"
			# create_table_mysql(files_table)
		end
	end
	def test_sub_funcs_in_mysql(funcs_table, file_name, function_name)
		if funcs_table == "" || file_name == ""
			raise ArgumentError, "#{__FILE__}, #{__LINE__}, params error"
		end
		tquery = DbConnect.new("#{$SIPDB}")
		tquery.with_db do |db|
			db.query("select * from #{funcs_table} where file_name='#{file_name}' and function_name='#{function_name}';") do |res|
				_file_exist = 0
				res.each_hash do |hash|
					hash.each do |k, v|
						#puts "#{k} : #{v}"
						if v == file_name
							_file_exist = 1
						end
					end
				end
				_file_exist
			end
		end
	end
	def select_file_name_from_table(table, params)
		sip_stack_name = params[:sip_stack_name]
		file_name = params[:file_name]
		function_name = params[:function_name]
		tquery = DbConnect.new("#{$SIPDB}")
		tquery.with_db do |db|
			db.query("select * from #{table} where file_name='#{file_name}'") do |res|
				res.each_hash do |hash|
					hash.each do |k, v|
						if k == "id"
							return v
						end
					end
				end
			end
		end
	end
	def insert_sub_funcs_to_mysql(params)
		sip_stack_name = params[:sip_stack_name]
		file_name = params[:file_name]
		function_name = params[:function_name]
		sub_functions_list = params[:sub_functions_list]
		sip_header_from_function = params[:sip_header_from_function]

		files_table = "#{sip_stack_name}_funcs_table"
		tquery = DbConnect.new("#{$SIPDB}")
		#debug_puts "insert into #{files_table}(file_name) values (\"#{file_name}\", \"#{function_list}\");"
		file_id = 0
		file_id = select_file_name_from_table("#{sip_stack_name}_files_table", params)
		#p file_id
		tquery.with_db do |db|
			db.query("insert into #{files_table}(file_id, file_name, function_name, sub_function_list, sip_header_from_function) 
					values (\'#{file_id}\', \'#{file_name}\', \'#{function_name}\', \'#{sub_functions_list}\', \'#{sip_header_from_function}\');") do |res|
			end
		end
	end
	def write_sub_funcs_to_mysql(params)
		sip_stack_name = params[:sip_stack_name]
		file_name = params[:file_name]
		function_name = params[:function_name]
		sub_functions_list = params[:sub_functions_list]
		# drop_table = params[:drop_table]
		if sip_stack_name == "" || file_name == ""
			raise ArgumentError, "#{__FILE__}, #{__LINE__}, params error"
		end
		_table_exist = 0
		funcs_table = "#{sip_stack_name}_funcs_table"
		#puts files_table
		_table_exist = test_table_in_mysql(funcs_table)
		#p _table_exist
		if _table_exist == 1
			_functions_exist = test_sub_funcs_in_mysql(funcs_table, file_name, function_name)
			if _functions_exist == 1
				#raise StandardError, "#{__FILE__}, #{__LINE__}, params error"
				log()
			else
				insert_sub_funcs_to_mysql(params)
			end
		else
			raise StandardError, "#{__FILE__}:#{__LINE__}:create_table error"
			# create_table_mysql(files_table)
		end
	end
end