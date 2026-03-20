select S.Name from Enrollment E 
join Student S 
on E.Student_ID=S.Student_ID 
join Course C 
on C.Course_ID=E.Course_ID 
where C.Course_Name like "%Engineering%" 
group by S.Student_ID 
having COUNT(DISTINCT C.Course_ID)=( 
	select COUNT(*) from Course 
	where Course_Name like "%Engineering%" 
);
