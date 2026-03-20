(select C.Course_Name,COUNT(*) from Course C 
left join Enrollment E 
on C.Course_ID=E.Course_ID 
group by C.Course_ID 
order by COUNT(*) desc 
limit 1) 

UNION 

(select C.Course_Name,COUNT(*) from Course C 
left join Enrollment E 
on C.Course_ID=E.Course_ID
group by C.Course_ID 
order by COUNT(*) asc 
limit 1);

