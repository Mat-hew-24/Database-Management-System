CREATE TABLE Department (
    Department_ID VARCHAR(10) PRIMARY KEY,
    Department_Name VARCHAR(50),
    Head_of_Dept_ID VARCHAR(10)
);

CREATE TABLE Faculty (
    Faculty_ID VARCHAR(10) PRIMARY KEY,
    Name VARCHAR(50),
    Department_ID VARCHAR(10),
    FOREIGN KEY (Department_ID) REFERENCES Department(Department_ID)
);

CREATE TABLE Student (
    Student_ID VARCHAR(10) PRIMARY KEY,
    Name VARCHAR(50),
    Date_of_Birth DATE,
    Email VARCHAR(50),
    Phone_Number INTEGER
);

CREATE TABLE Course (
    Course_ID VARCHAR(10) PRIMARY KEY,
    Course_Name VARCHAR(50),
    Department_ID VARCHAR(10),
    Faculty_ID VARCHAR(10),
    FOREIGN KEY (Department_ID) REFERENCES Department(Department_ID),
    FOREIGN KEY (Faculty_ID) REFERENCES Faculty(Faculty_ID)
);

CREATE TABLE Enrollment (
    Student_ID VARCHAR(10),
    Course_ID VARCHAR(10),
    Grade INTEGER,
    PRIMARY KEY (Student_ID, Course_ID),
    FOREIGN KEY (Student_ID) REFERENCES Student(Student_ID),
    FOREIGN KEY (Course_ID) REFERENCES Course(Course_ID)
);

ALTER TABLE Department
ADD FOREIGN KEY (Head_of_Dept_ID) REFERENCES Faculty(Faculty_ID);
