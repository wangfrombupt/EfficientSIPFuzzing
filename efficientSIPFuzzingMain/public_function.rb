##public_function.rb

module PublicFunction
	def log(message="")
	end
	def write_to_yaml(_file_name = "", *params)
		if _file_name == ""
			raise ArgumentError, "error"
		end
		File.open(_file_name, 'w') do |f|
			YAML.dump(params, f)
		end
	end
	
end