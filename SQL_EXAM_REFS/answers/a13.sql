select S.Name from Student S 
left join Enrollment E 
on E.Student_ID=S.Student_ID 
group by S.Student_ID 
having COUNT(E.Course_ID)=0;
