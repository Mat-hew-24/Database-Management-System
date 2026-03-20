SELECT S.Name FROM Student S 
JOIN Enrollment E 
ON E.Student_ID = S.Student_ID 
JOIN Course C 
ON C.Course_ID = E.Course_ID
JOIN Department D
ON D.Department_ID=C.Department_ID
WHERE D.Department_Name
IN ('Mathematics','Physics') 
GROUP BY S.Name;
