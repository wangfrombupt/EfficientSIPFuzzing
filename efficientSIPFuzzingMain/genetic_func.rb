module GenFunc
	@@format_value = []
	@@format_value << ['0XC00X80', '0XE00X800X80', '0XF00X800X800X80', '0XFC0X800X800X800X800X80']
	@@format_value << ['%s', '%x', '%n', '%n', '%x', '%*s', '%.*s', '%.127d', '%*.*s', '%.555d', '%*d', 's%']
	@@format_value << "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\/:-@=\."
	@@format_value << ['0X1B', '0X00', '\\0X00']
	def gen_values_by_report(tem_value, tem_class)
		res = []
		if(tem_class == "10")
			# format overflow
			res << "{'class'=>'10', 'value'=>'%s(#{1024*2+rand(10240)})'}"
			res << "{'class'=>'10', 'value'=>'%s(#{1024*2+rand(10240)})'}"
		end

		if(tem_class == "20")
			# general overflow
		end

		if(tem_class == "30")
			# number
			res << "{'class'=>'30', 'value'=>'-#{rand(65535)}'}"
			res << "{'class'=>'30', 'value'=>'-#{rand(65535)}'}"
		end
		res
	end

	def gen_values_by_report_1(tem_value, tem_class)
		res = []
		if(tem_class == "10")
			# format overflow
			tem_value =~ /(.*)\((.*)\)/
			_v = $1
			_t = $2.to_i
			_f_str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@\#\{$%^&*()\},.<>?/:;\'\'\"[]"
			1.times do |i|
				r_v = "%#{_f_str[rand(78)]}"
				res << "{'class'=>'10', 'value'=>'#{r_v}(#{_t+rand(_t)})'}"
				r_v = _v
				_v.size.times do |i|
					r_v = r_v + "%#{_f_str[rand(78)]}"
				end
				_tt = _t / _v.size
				res << "{'class'=>'10', 'value'=>'#{r_v}(#{_tt})'}"
			end
			res << "{'class'=>'10', 'value'=>'#{_v}(#{_t+rand(_t)})'}"
		end

		if(tem_class == "20")
			# general overflow
			tem_value =~ /(.*)\((.*)\)/
			_v = $1
			_t = $2.to_i
			_f_str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@\#\{$%^&*()\},.<>?/:;\'\'\"[]"
			1.times do |i|
				r_v = "#{_f_str[rand(78)]}"
				res << "{'class'=>'20', 'value'=>'#{r_v}(#{_t+rand(_t)})'}"
				r_v = _v
				_v.size.times do |i|
					r_v = r_v + "#{_f_str[rand(78)]}"
				end
				_tt = _t / _v.size
				res << "{'class'=>'20', 'value'=>'#{r_v}(#{_tt})'}"
			end
			res << "{'class'=>'20', 'value'=>'#{_v}(#{_t+rand(_t)})'}"
		end

		if(tem_class == "30")
			# number
			if(tem_value.to_i < 0)
				res << "{'class'=>'30', 'value'=>'-#{rand(65535)}'}"
				res << "{'class'=>'30', 'value'=>'-#{rand(65535)}'}"
			else
				res << "{'class'=>'30', 'value'=>'#{rand(65535)}'}"
				res << "{'class'=>'30', 'value'=>'#{rand(65535)}'}"
			end
		end
		res
	end

	def gen_values_by_report_2(tem_value, tem_class)
		res = []
		if(tem_class == "10")
			# format overflow
			tem_value =~ /(.*)\((.*)\)/
			_v = $1
			_t = $2.to_i
			_f_str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@\#\{$%^&*()\},.<>?/:;\'\'\"[]"
			1.times do |i|
				r_v = "%#{_f_str[rand(78)]}"
				res << "{'class'=>'10', 'value'=>'#{r_v}(#{_t})'}"
				r_v = _v
				_v.size.times do |i|
					r_v = r_v + "%#{_f_str[rand(78)]}"
				end
				_tt = _t / (r_v.size/_v.size)
				res << "{'class'=>'10', 'value'=>'#{r_v}(#{_tt})'}"
			end
			res << "{'class'=>'10', 'value'=>'#{_v}(#{_t})'}"
		end

		if(tem_class == "20")
			# general overflow
			tem_value =~ /(.*)\((.*)\)/
			_v = $1
			_t = $2.to_i
			_f_str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@\#\{$%^&*()\},.<>?/:;\'\'\"[]"
			1.times do |i|
				r_v = "#{_f_str[rand(78)]}"
				res << "{'class'=>'20', 'value'=>'#{r_v}(#{_t})'}"
				r_v = _v
				_v.size.times do |i|
					r_v = r_v + "#{_f_str[rand(78)]}"
				end
				_tt = _t / (r_v.size/_v.size)
				res << "{'class'=>'20', 'value'=>'#{r_v}(#{_tt})'}"
			end
			res << "{'class'=>'20', 'value'=>'#{_v}(#{_t})'}"
		end

		if(tem_class == "30")
			# number
			if(tem_value.to_i < 0)
				res << "{'class'=>'30', 'value'=>'-#{rand(65535)}'}"
				res << "{'class'=>'30', 'value'=>'-#{rand(65535)}'}"
			else
				res << "{'class'=>'30', 'value'=>'#{rand(65535)}'}"
				res << "{'class'=>'30', 'value'=>'#{rand(65535)}'}"
			end
		end
		res
	end

	@@index_arr = []
	@@tmp_arr = []
	def c(n, m, s=0, k=0)
		if k == m
			eval "@@index_arr << #{@@tmp_arr.to_s}"
		else
			for i in s...n
				@@tmp_arr.push(i)
				c(n, m, i+1, k+1)
				@@tmp_arr.pop
			end
		end
	end

	def gen_values_by_report_2_fixed_length(tem_value, tem_class, len=1024, type_cnt = 2)
		res = []
		# => zuhe shu C(@@format_value, type_cnt)
		
		c(@@format_value.length, type_cnt)
		p @@index_arr
		res_str = ""

		@@index_arr.each do |ele|
			res_str = ""
			while res_str.length < len
				ele.each do |i|
					res_str << @@format_value[i][rand(@@format_value[i].length)]
				end
			end
			res << "{'class'=>'10', 'value'=>'#{res_str}'}"
		end
		res
	end

	def gen_values_by_report_2_fixed_type(tem_value, tem_class, base=1024, step=5000, type=1)
		res = []
		
		@@format_value.each do |ele|
			res_str = ""
			while res_str.length < base
				res_str << ele[rand(ele.length)]
			end
			res << "{'class'=>'10', 'value'=>'#{res_str}'}"
		end
		res
	end

	def gen_format(tem_value)
		_f_str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@\#\{$%^&*()\},.<>?/:;\'\'\"[]"
		# %[flag][width][.perc][F|N|h|l]type
		p tem_value
		tem_value =~ /%([-+0])?(\d*)?([.])?(\d*)?([FNhl])?(.*)$/
		_flag = $1
		_width = $2
		_dot = $3
		_perc = $4
		_fnhl = $5
		_type = $6

		p $1
		p $2
		p $3
		p $4
		p $5
		p $6


		if(_flag == nil)
			_flag = "-+0"[rand(3)]
		end
		if(_width == nil || _width == '')
			_width = rand(1000)
		else
			_width = _width.to_i+rand(_width.to_i)
		end

		if(_dot == nil)
			_dot = '.'
		end

		if(_perc == nil || _perc == '')
			_perc = rand(1024)
		else
			_perc = _perc.to_i + rand(_perc.to_i)
		end

		if(_fnhl == nil)
			_fnhl = nil
		else
			_fnhl = nil
		end

		if(_type == nil || _type == '')
			_type = 's'
		else
			_type = _f_str[rand(_f_str.size)]
		end


		res = '%'+_flag+_width.to_s+_dot+_perc.to_s+_type

		p res
		
	end
	def gen_values_by_report_3(tem_value, tem_class)
		res = []
		_rstr = ""
		_len = tem_value.size
		i=0
		while i < _len
			case tem_value[i]
			when "%"
				t_str = tem_value[i.._len]
				t_str =~ /(.(.*?))([%(]|$)/
				c_str = $1
				_rstr = _rstr + gen_format(c_str)
			end
			i = i+1
		end
		res
	end

end

include GenFunc

res = gen_values_by_report_2("%s(10240)", "10")
p res