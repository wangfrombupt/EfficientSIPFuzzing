require 'mydebug'

module LeakAnalyze
	class LeakBaseInfo
		include MyDebug

		def initialize(_sip_stack_name="opensbc", _sip_stack_version="1.1.5.42", _error_file_name="", 
			_error_function_name="", _error_begin_raw=0, _error_end_raw=0, *_other_info)
			super()
			@sip_stack_name = _sip_stack_name
			@sip_stack_version = _sip_stack_version
			@error_file_name = _error_file_name
			@error_function_name = _error_function_name
			@error_begin_raw = _error_begin_raw
			@error_end_raw = _error_end_raw
			@other_info = _other_info
			# debug_puts @other_info
		end
		def to_s
			"sip stack name: #{@sip_stack_name}\n" +
			"sip stack version: #{@sip_stack_version}\n" +
			"error file name: #{@error_file_name}\n" +
			"error function name: #{@error_function_name}\n" +
			"error begin raw: #{@error_begin_raw}\n" +
			"error end raw: #{@error_end_raw}\n" +
			"other info:#{@other_info.to_s}\n"
		end
	end
end

include LeakAnalyze
leak = LeakBaseInfo.new('opensips', '1.10.0', "sipVia", "parse_to", 10, 20, "strcpy", "sprintf")
puts leak
