select C.* from Enrollment E 
left join Course C 
on C.Course_ID=E.Course_ID 
group by C.Course_ID 
having MAX(E.Grade)-MIN(E.Grade) = ( 
	select MAX(Maximum - Minimum) from ( 
		select MAX(Grade) as Maximum,MIN(Grade) as Minimum from Enrollment 
		group by Course_ID
	) as SUB 
);
