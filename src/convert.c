

int pet2a(int c){	
	c&=0xff;
	if(c>=65 && c<=90) {
		c+=32;
	} else 
	if(c>=193 && c<=218) {
		c&=0x7f;
	}
	return(c);
}

int a2pet(int c){
	c&=0xff;
	if(c>=65 && c<=90) {
		c|=0x80;
	} else 
        if(c>=97 && c<=122) {
                c-=32;
        }			
	return(c);
}

