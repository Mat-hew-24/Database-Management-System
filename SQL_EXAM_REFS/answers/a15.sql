select DISTINCT C.Course_Name from Course C 
join Enrollment E 
on C.Course_ID=E.Course_ID 
where E.Grade < 6;
