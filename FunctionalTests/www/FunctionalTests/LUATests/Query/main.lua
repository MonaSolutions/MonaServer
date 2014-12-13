_tab1 = { key1="val1" , key2="val2" }
_str = "str#ing"
_tab2 = { key3="val3", key4=10 , key5=true}

function run()
	local query = mona:toQuery(_tab1,_str,_tab2)
	assert(query=="key1=val1&key2=val2&str%23ing&key3=val3&key5=true&key4=10") -- order change
	local tab1,str,tab2 = mona:fromQuery(query)
	compareTable(tab1,_tab1)
	assert(str,_str)
	compareTable(tab2,_tab2)
end
