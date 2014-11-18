class ValueProcess
	def initialize(str)
		@strRp=str
	end

	#将十六进制数字进行转换,返回转换字符串
	def getElement(element)
		finalStr=''
		location=element.index('0x')
		#若没有十六进制标识（0x）直接返回原字符串
		if location==nil
			return element
		#若含有十六进制标识（0x）对所有的十六进制标识进行处理，主要函数是ord和chr
		else
			while location!=nil
			if (element[location+2,1]>='a')&(element[location+2,1]<='f')
				first=element[location+2].to_i-'a'[0].to_i+10
			else
				first=element[location+2].to_i-'0'[0].to_i
			end

			if (element[location+3,1]>='a')&(element[location+3,1]<='f')
				second=element[location+3].to_i-'a'[0].to_i+10
			else
				second=element[location+3].to_i-'0'[0].to_i
			end

			result=(first*16+second).chr
			finalStr+=result
			location=element.index('0x',location+2)
			end
		end
		return finalStr
	end
#	getElement("0xe00x80")



	#根据重复次数得到长字符串；对于非规则的重复内容，要先进行转换（使用getElement函数）
	#返回替换字符串
	def transValue()
		finalStr=''
		#empty，返回空字串
		if @strRp=='empty'
			return finalStr
		end

		#左括号的位置
		llocation=@strRp.index('(')
		#右括号的位置
		rlocation=-1
		#若没有左括号，则说明不需要进行重复字符的处理 例如overflow-general的value为a(3)
		if llocation==nil
			finalStr=getElement(@strRp)
			return finalStr
		else
			tmp=''
			content=''
			times=''
			count=0
			while llocation!=nil
				tmp=@strRp[0,llocation]
				content=getElement(tmp)
				rlocation=@strRp.index(')',llocation)
				num_len=rlocation-llocation-1
				times=@strRp[(llocation+1),num_len]
				count = times.to_i
				i=0
				while i<count
					finalStr=finalStr << content
					i=i+1
				end
				llocation=@strRp.index('(',rlocation)
			end
			#如果最后一个右括号后还有字符**，**若非数字直接添加在finalStr后面[a.(3)com]，**若为数字finalStr重复**遍[a(1)0x00(1)127]
			repeat_num=@strRp.length-rlocation-1
			if repeat_num==0
				return finalStr
			else
				tmp=@strRp[(rlocation+1),repeat_num]
				tmp2=getElement(tmp)
				if (tmp2[0,1]>='0')&(tmp2[0,1]<='9')
					count1 = tmp.to_i
					tmp2=finalStr
					i=0
					#原来的finalStr非空，所以只需要为它加count-1次
					while i<(count-1)
					finalStr+=tmp2
					i=i+1
					end
				else
					finalStr+=tmp2
				end
			end
		end
		return finalStr
	end
end

# test = ValueProcess.new("")
# p test.getElement("0xe00x80")