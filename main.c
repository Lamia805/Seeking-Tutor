#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#define Max_stu 2000
int com=0;
int totalReq=0;
int totalSes=0;
int tutoring=0;
void *student_thread(void *student_id);
void *tutor_thread(void *tutor_id);
void *coordinator_thread();
int student_num=0;
int tutor_num=0;
int help_num=0;
int chair_num=0;
int occupied_chair_num=0;
int newArrivedStudentQueue[Max_stu];
int tutorFinishedQueue[Max_stu];
int priorityQueue[Max_stu][2];
int student_priority[Max_stu];
int student_ids[Max_stu];
int tutor_ids[Max_stu];
sem_t sem_student;
sem_t sem_coordinator;
pthread_mutex_t seatLock;
pthread_mutex_t queueLock;



pthread_mutex_t tutorFinishedQueueLock;
int main(int argc, const char * argv[]) {
if (argc != 5){
printf("Usage: <# of Students> <# of tutors> <# of chairs> <# of help>\n");
exit(-1);
}
student_num=atoi(argv[1]);
tutor_num=atoi(argv[2]);
chair_num=atoi(argv[3]);
help_num=atoi(argv[4]);
if(student_num > Max_stu || tutor_num > Max_stu){
printf("Max student number is: %d; Max tutor number is: %d\n", Max_stu,
Max_stu);
exit(-1);
}
int i;
for(i=0;i<student_num;i++){
newArrivedStudentQueue[i]=-1;
tutorFinishedQueue[i]=-1;
priorityQueue[i][0]=-1;
priorityQueue[i][1]=-1;
student_priority[i]=0;
}
sem_init(&sem_student,0,0);
sem_init(&sem_coordinator,0,0);
pthread_mutex_init(&seatLock,NULL);
pthread_mutex_init(&queueLock,NULL);
pthread_mutex_init(&tutorFinishedQueueLock,NULL);
pthread_t students[student_num];
pthread_t tutors[tutor_num];
pthread_t coordinator;



assert(pthread_create(&coordinator,NULL,coordinator_thread,NULL)==0);
for(i = 0; i < student_num; i++)
{
student_ids[i] = i + 1;
assert(pthread_create(&students[i], NULL, student_thread, (void*)
&student_ids[i])==0);
}
for(i = 0; i < tutor_num; i++)
{
tutor_ids[i] = i + student_num + 1;
assert(pthread_create(&tutors[i], NULL, tutor_thread, (void*)
&tutor_ids[i])==0);
}
pthread_join(coordinator, NULL);
for(i =0; i < student_num; i++)
{
pthread_join(students[i],NULL);
}
for(i =0; i < tutor_num; i++)
{
pthread_join(tutors[i],NULL);
}
return 0;
}
void *student_thread(void *student_id){
int id_student=*(int*)student_id;
while(1){
if(student_priority[id_student-1]>=help_num) {


pthread_mutex_lock(&seatLock);
com++;
pthread_mutex_unlock(&seatLock);
sem_post(&sem_student);
pthread_exit(NULL);
}
float programTime=(float)(rand()%200)/100;
usleep(programTime);
pthread_mutex_lock(&seatLock);
if(occupied_chair_num>=chair_num){
printf("St: Student %d found no empty chair. Will try again later.\n",id_student);
pthread_mutex_unlock(&seatLock);
continue;
}
occupied_chair_num++;
totalReq++;
newArrivedStudentQueue[id_student-1]=totalReq;
printf("St: Student %d takes a seat. Empty chairs = %d\n",id_student,chair_num-occupied_chair_num);
pthread_mutex_unlock(&seatLock);
sem_post(&sem_student);
while(tutorFinishedQueue[id_student-1]==-1);
printf("St: Student %d received help from Tutor %d.\n",id_student,tutorFinishedQueue[id_student-1]-student_num);
pthread_mutex_lock(&tutorFinishedQueueLock);
tutorFinishedQueue[id_student-1]=-1;
pthread_mutex_unlock(&tutorFinishedQueueLock);



pthread_mutex_lock(&seatLock);
student_priority[id_student-1]++;
pthread_mutex_unlock(&seatLock);
}
}
void *tutor_thread(void *tutor_id){
int id_tutor=*(int*)tutor_id;
int studentTutoredTimes;
int studentSequence;
int id_student;
while(1){
if(com==student_num){
pthread_exit(NULL);
}
studentTutoredTimes=help_num-1;
studentSequence=student_num*help_num+1;
id_student=-1;
sem_wait(&sem_coordinator);
pthread_mutex_lock(&seatLock);
int i;
for(i=0;i<student_num;i++){
if(priorityQueue[i][0]>-1 && priorityQueue[i][0]<=studentTutoredTimes
&& priorityQueue[i][1]<studentSequence){
studentTutoredTimes=priorityQueue[i][0];
studentSequence=priorityQueue[i][1];
id_student=student_ids[i];
}
}
if(id_student==-1) {
pthread_mutex_unlock(&seatLock);
continue;



}
priorityQueue[id_student-1][0]=-1;
priorityQueue[id_student-1][1]=-1;
occupied_chair_num--;
tutoring++;
pthread_mutex_unlock(&seatLock);
float tutorTime=(float)(rand()%200)/1000;
usleep(tutorTime);
pthread_mutex_lock(&seatLock);
tutoring;
totalSes++;
printf("Tu: Student %d tutored by Tutor %d. Students tutored now = %d.Total sessions tutored = %d\n",id_student,id_tutor-student_num,tutoring,totalSes);

pthread_mutex_unlock(&seatLock);
pthread_mutex_lock(&tutorFinishedQueueLock);
tutorFinishedQueue[id_student-1]=id_tutor;
pthread_mutex_unlock(&tutorFinishedQueueLock);
}
}
void *coordinator_thread(){
while(1){
if(com==student_num){
int i;
for(i=0;i<tutor_num;i++){
sem_post(&sem_coordinator);
}
pthread_exit(NULL);
}
sem_wait(&sem_student);



pthread_mutex_lock(&seatLock);
int i;
for(i=0;i<student_num;i++){
if(newArrivedStudentQueue[i]>-1){
priorityQueue[i][0]=student_priority[i];
priorityQueue[i][1]=newArrivedStudentQueue[i];
printf("Co: Student %d with priority %d in the queue. Waiting students now = %d. Total requests = %d\n",student_ids[i],student_priority[i],occupied_chair_num,totalReq);
newArrivedStudentQueue[i]=-1;
sem_post(&sem_coordinator);
}
}
pthread_mutex_unlock(&seatLock);
}
}
