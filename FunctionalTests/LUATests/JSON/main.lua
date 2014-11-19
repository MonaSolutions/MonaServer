local tab = {
  {10,10,100,100},
  {x=10,y=10,width=100,height=100},
  {x=10,y=10,100,100},
  {x={10,10,10,10}},
  {x={width=100,height=100,100,100}},
  {x={100,100,width=100,height=100}},
  {a=1,b=2,3,4,x={{{1,2,3}}}}
}

function run()
	assert(mona:toJSON(tab[1]) == '[[10,10,100,100]]')
	assert(mona:toJSON(tab[2]) == '[{"y":10,"x":10,"height":100,"width":100}]') -- order change
	assert(mona:toJSON(tab[3]) == '[[{"y":10,"x":10},100,100]]')  -- order change
	assert(mona:toJSON(tab[4]) == '[{"x":[10,10,10,10]}]')
	assert(mona:toJSON(tab[5]) == '[{"x":[{"height":100,"width":100},100,100]}]')  -- order change
	assert(mona:toJSON(tab[6]) == '[{"x":[{"height":100,"width":100},100,100]}]')  -- order change
	assert(mona:toJSON(tab[7]) == '[[{"b":2,"x":[[[1,2,3]]],"a":1},3,4]]')  -- order change
	
	compareTable(tab[1],mona:fromJSON('[[10,10,100,100]]'))
	compareTable(tab[2],mona:fromJSON('[{"x":10,"y":10,"width":100,"height":100}]'))
	-- compareTable(tab[3],mona:fromJSON('[[{"x":10,"y":10},100,100]]'))
	compareTable(tab[4],mona:fromJSON('[{"x":[10,10,10,10]}]'))
	-- compareTable(tab[5],mona:fromJSON('[{"x":[{"width":100,"height":100},100,100]}]'))
	-- compareTable(tab[6],mona:fromJSON('[{"x":[100,100,{"width":100,"height":100}]}]'))
	-- compareTable(tab[7],mona:fromJSON('[[{"a":1,"b":2,"x":[[[1,2,3]]]},3,4]]'))
end