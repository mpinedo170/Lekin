#ifndef T_INDEX_H
#define T_INDEX_H

#include "const.h"
#include "lib_func.h"

#define NULL_INDEX_VALUE \
    -1  // Used for initializing.
        // If we let negative value indices, this number should be decresed.

template <class T>
class Beam_Indices;

template <class T>
class Class_Index_Element
{
    friend class Beam_Indices<T>;

    double value;  // The value of the index
    int location;  // The location of the element.

    T* p_element;  // This structure is particular to any case.

public:
    Class_Index_Element()
    {
        value = NULL_INDEX_VALUE;
        // Only nonegative indices are allowed.

        location = NULL_LOCATION;
        p_element = NULL;
    };

    typedef Class_Index_Element<T>* type_p_Class_Index_Element;
};

template <class T>
class Beam_Indices
{
    // Indices part:

    int number_elements;
    // Maximum number of elements to be processed.

    Class_Index_Element<T>* array_Class_Index;
    // Store the value of the index and its element.
    // It is usefull for the branching.

    int* best_array_Class_Index;
    // It is an index of the array above.
    // Its first entry contains the entry number in array_Class_Index[];
    // that has the best (highest) value, and so on.

    int number_elements_list;
    // Number of elements (and its indices) already added to the list.
    // It grows as the list gets bigger.

    // Branching part:
    int number_branched_so_far;
    // Number of branches scanned so far.

    int max_branches;
    // maximum number of branches the list will allow.
    // CHECK! may be combined with number_elements.

    double threshold;
    // If the value of a solution is less than threshold*optimal_value,
    // then it is not returned in Next_Best().

    int number_non_zero_elements;
    // Number of elements that have not a zero index.

public:
    // Functions:

    Beam_Indices(){};

    void Initialize(int input_number_elements, int input_max_branches, double input_threshold)
    {
        int i;

        number_elements = input_number_elements;

        // Allocating memory:

        array_Class_Index = new Class_Index_Element<T>[number_elements];

        best_array_Class_Index = new int[number_elements];

        // Indices part:
        number_elements_list = 0;
        for (i = 0; i < number_elements; i++)
        {
            best_array_Class_Index[i] = 0;
            array_Class_Index[i].p_element = NULL;
        }

        // Branching part:

        max_branches = input_max_branches;
        threshold = input_threshold;
        number_branched_so_far = 0;
        number_non_zero_elements = 0;
    };

    void Terminate()
    {
        int i;

        // De-allocating memory:

        for (i = 0; i < number_elements_list; i++)
        {
            if (array_Class_Index[i].p_element != NULL)
            {
                array_Class_Index[i].p_element->Terminate();
                // This is for the solution oracle, not for the solution increment.

                delete array_Class_Index[i].p_element;
            }
#ifdef CHECK_DEBUG
            else
            {
                printf("ERROR: terminating a non null element \n");
                exit(1);
            }
#endif
        }
        //                delete[] array_Class_Index ;     // commented out by Andrew!
        delete[] best_array_Class_Index;
    }

    // Worth_Adding() returns TRUE if the element to be added is promissing enough.
    int Worth_Adding(double input_objective_value)
    {
        if (number_elements_list == 0)
            return (TRUE);
        else
        {
            if (input_objective_value < (threshold * array_Class_Index[best_array_Class_Index[0]].value))
                // This is a heuristic.
                // If the objective value is too small with respect to the best one, desregard it.

                return (FALSE);
            else
                return (TRUE);
        }
    }

    void Add_Element(double added_index, T* p_input_element, int input_location)
    {
        int i = 0;
        int j;

#ifdef CHECK_DEBUG
        assert(added_index >= 0);
#endif

        // Here we implement an insertion algorithm.
        // The first element is the biggest number.

        // i is the position that the new index should have in  (*p_best_array_Class_Index)[]
        while ((i < number_elements_list) && (i < number_elements) &&
               (added_index <= array_Class_Index[best_array_Class_Index[i]]
                                   .value))  //"<=" since if it has the same value, it should be placed to the
                                             //right of the previous index.
            i++;

        if (i < number_elements)
        {
            // If i is too big, desregard it.

            if (number_elements_list < number_elements)
            {
                array_Class_Index[number_elements_list].value = added_index;
                array_Class_Index[number_elements_list].location = input_location;
                array_Class_Index[number_elements_list].p_element = p_input_element;
                // Adding the element to the last place in the list.

                number_elements_list++;
                if (added_index > 0) number_non_zero_elements++;

                j = min2(number_elements_list - 1, number_elements - 1);
                while (j > i)
                {
                    best_array_Class_Index[j] = best_array_Class_Index[j - 1];
                    j--;
                }
                best_array_Class_Index[j] = number_elements_list - 1;
            }
            else
            {
                if ((array_Class_Index[best_array_Class_Index[number_elements - 1]].value == 0) &&
                    (added_index > 0))
                    number_non_zero_elements++;

                int updating_position = best_array_Class_Index[number_elements - 1];
                array_Class_Index[best_array_Class_Index[number_elements - 1]].value = added_index;
                array_Class_Index[best_array_Class_Index[number_elements - 1]].location = input_location;
                array_Class_Index[best_array_Class_Index[number_elements - 1]].p_element = p_input_element;
                // Adding the element to the place of the worst element.

                j = number_elements - 1;
                while (j > i)
                {
#ifdef CHECK_DEBUG
                    assert(best_array_Class_Index[j] < number_elements_list);
#endif
                    best_array_Class_Index[j] = best_array_Class_Index[j - 1];
                    j--;
                }
                best_array_Class_Index[j] = updating_position;
            }
        }
    }

    void Next_Best(double* p_index, int* p_location, T** p_p_output_element)
    {
#ifdef CHECK_DEBUG
        assert(number_branched_so_far < max_branches);
#endif

        *p_index = array_Class_Index[best_array_Class_Index[number_branched_so_far]].value;
        *p_location = array_Class_Index[best_array_Class_Index[number_branched_so_far]].location;

        *p_p_output_element = array_Class_Index[best_array_Class_Index[number_branched_so_far]].p_element;
        number_branched_so_far++;
    }

    void Get_Number_Schedulable(int* p_number_schedulable)
    {
        int i = 0;
        double best_index;

#ifdef CHECK_DEBUG
        assert(number_elements_list >= 1);
#endif

        best_index = array_Class_Index[best_array_Class_Index[0]].value;
        while ((i < number_elements_list) &&
               (array_Class_Index[best_array_Class_Index[i]].value >= threshold * best_index))
        {
            i++;
        }
        *p_number_schedulable = i;
    }

    int Highest_Id()
    {
        return (array_Class_Index[best_array_Class_Index[0]].location);
    }

    // Returns the pointer to the element in the ith position.
    T* Get_p_ith_Element(int i)
    {
        return (array_Class_Index[best_array_Class_Index[i]].p_element);
    }
    int Get_ith_Location(int i)
    {
        return (array_Class_Index[best_array_Class_Index[i]].location);
    }
    int Number_Elements()
    {
        return (number_elements);
    }
    int Number_Elements_List()
    {
        return (number_elements_list);
    }
    int* Get_Best_Array_Class_Index()
    {
        return (best_array_Class_Index);
    }
    int Number_Non_Zero_Elements()
    {
        return (number_non_zero_elements);
    }
    int Is_Member(double input_value, T* p_element)
    {
        if (number_elements_list == 0) return (FALSE);
        int i = 0;
        while (i < number_elements_list)
        {
            if (array_Class_Index[best_array_Class_Index[i]].value < input_value)
                return (FALSE);
            else if (array_Class_Index[best_array_Class_Index[i]].value == input_value)
                if (*p_element == *(array_Class_Index[best_array_Class_Index[i]].p_element)) return (TRUE);
            i++;
        }

        return (FALSE);
    }
};

#endif
