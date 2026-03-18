select S.Name from Student S 
join Enrollment E 
on E.Student_ID=S.Student_ID 
join Course C 
on E.Course_ID=C.Course_ID 
where C.Course_Name in ("Linear Algebra","Nuclear Physics") 
group by S.Student_ID 
having COUNT(DISTINCT C.Course_Name)=2;
