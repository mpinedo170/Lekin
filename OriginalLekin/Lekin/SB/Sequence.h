#pragma once

#include "Parameter.h"
#include "Stack.h"

/* TSequence class

   This object contains machine id and its sequence.
   The format is as follows.
   Batch size == 1
   ---------------
     mID -> oID1 -> oID2 -> oID3 -> .....

   Batch size > 1
   --------------
     mID -> oID1 -> oID2 -> 0 -> oID3 -> 0 -> ....

     note: 0 means the processing start

   Commands
   ========
   TSequence(TMachineID)
 x [i]           : return the operation id at location i.
 - <<oID         : append operation id=oID.
 - <<sq          : append sequence sq to this sequence
 - SetID(xx)     : set machine id.
 - Append(oID)   : add operation id=xx to last item in the list.
 - Append(sq)    : add sequence sq to this sequence.
 b Remove(x)     : remove the xth operation; return 1/0.
 - Clear()       : clear the sequence.
 i GetLast()     : return ID of the last operation on the sequence
*/
class TSequence : public TStack
{
public:
    TMachineID mID;

    TSequence(TMachineID mIDI = 0)
    {
        mID = mIDI;
    };
    TSequence(TSequence& seqX);
    TSequence& operator=(TSequence& seqX);
    int operator==(TSequence& seqX);
    TSequence& operator<<(TOperationID oIDI)
    {
        Append(oIDI);
        return *this;
    }
    TSequence& operator<<(TSequence& szX)
    {
        Append(szX);
        return *this;
    }
    void SetID(TMachineID _mID)
    {
        Clear();
        mID = _mID;
    }
    void Append(TOperationID oIDI)
    {
        Push(oIDI);
    }
    void Append(TSequence& szX);
    int Remove(int iLocation)
    {
        return Delete(iLocation);
    }
    void Remove0();
    TOperationID GetLast()
    {
        int iN = Num();
        return iN ? (TOperationID)(*this)[iN] : 0;
    }
};

/* TSequenceList class

   This class contains the sequences for machines in the shop. Two types of
   disjunctive graphs are in the data members. This will let the user to
   get the information on completion time and duedate related info.
   Commands
   ========
   TSequenceList(),TSequenceList(joblist)
 x [i]              : return sequence at ith location.
 - Append(mID,oID)  : append operation to machine "mID". If the sequence
                      does not exist, create a new one.
 - Append(mID,sx)   : append TStack_oID "sx" to machine "mID"
 - <<sq             : append sequence "sq".
 p Append(sq)       : append sequence "sq".
 - Append(sql)      : append list of sequences.
 p Get(x)           : return the ptr to the sequence at xth location.
 p Gets_mID(y)      : return the ptr to the sequence of machine ID=y.
 p Gets_oID(y)      : return ptr to sequence that contain the operation y.
 b Remove(x)        : remove the sequence at xth location.
 b Remove(p)        : remove the sequence that ptr 'p' point to.
 b RemoveID(y)      : remove the sequence of machine ID=y.
 - Clear()          : remove all the sequence.
 i GetLast(mID)     : return oID of the last operation on the sequence of
                      machine mID, return 0 if not found

*/
class TSequenceList : public TStackP
{
public:
    TSequenceList()
    {}
    TSequenceList(TSequenceList& sql);
    TSequenceList& operator=(TSequenceList& sql);
    TSequence& operator[](int iLocation)
    {
        return *(TSequence*)TStackP::operator[](iLocation);
    }
    void Append(TMachineID mIDI, TOperationID oIDI);
    void Append(TMachineID mIDI, TStack stkOper);
    void Append(TSequenceList& sqlX);
    TSequence* Append(TSequence& sqX);
    void operator<<(TSequence& sqX)
    {
        Append(sqX);
    }
    void operator<<(TSequenceList& sqlX)
    {
        Append(sqlX);
    }
    TSequence* Get(int iLocation);
    TSequence* Gets_mID(TMachineID mIDI);
    TSequence* Gets_oID(TOperationID oIDI);
    int Remove(int iLocation);
    int Remove(TSequence* psqX);
    int RemoveID(TMachineID mIDI);
    void Clear();
    TOperationID GetLast(TMachineID mID);
};

//*** overload iostream ***//
ostream& operator<<(ostream& os, TSequence& seqX);
ostream& operator<<(ostream& os, TSequenceList& sqlX);

//*** overload istream ***//
istream& operator>>(istream& is, TSequenceList& sqlX);

void Trace(LPSTR szName, TSequenceList& sql);
void Trace(LPSTR szName, TSequence& sq);
