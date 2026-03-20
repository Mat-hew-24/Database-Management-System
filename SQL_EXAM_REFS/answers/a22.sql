select S.Name from Student S 
left join Enrollment E 
on E.Student_ID=S.Student_ID 
left join Course C 
on C.Course_ID=E.Course_ID  
where C.Department_ID IN ( 
	select C1.Department_ID from Course C1
	left join Enrollment E1 
	on E1.Course_ID=C1.Course_ID 
	group by C1.Department_ID
	having AVG(E1.Grade) = ( 
		select MIN(avgGrade) from ( 
			select AVG(Grade) as avgGrade from Course C4
			join Enrollment E2
			on E2.Course_ID=C4.Course_ID 
			group by C4.Department_ID
		) AS SUB1
	)
) 
group by S.Student_ID 
having COUNT(DISTINCT C.Course_ID) = ( 
	select COUNT(*) from Course C2
	where C2.Department_ID IN ( 
		select C3.Department_ID from Course C3
		left join Enrollment E3 
		on E3.Course_ID=C3.Course_ID 
		group by C3.Department_ID
		having AVG(E3.Grade) = ( 
			select MIN(avgGrade) from ( 
				select AVG(Grade) as avgGrade from Course C5
				join Enrollment E4 on E4.Course_ID=C5.Course_ID 
				group by C5.Department_ID
			) AS SUB2
		)
	) 
);
	
