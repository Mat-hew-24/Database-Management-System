select S.Name from Student S 
join Enrollment E 
on E.Student_ID=S.Student_ID 
join Course C 
on E.Course_ID=C.Course_ID
join Department D
on D.Department_ID=C.Department_ID
where D.Department_Name in ('Mathematics','Physics') 
group by S.Student_ID 
having COUNT(DISTINCT D.Department_ID)=2;
