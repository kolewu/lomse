//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_note_engraver.h"
#include "lomse_chord_engraver.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
#include "lomse_im_factory.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// Helper. Access to protected members
class MyChordEngraver : public ChordEngraver
{
public:
    MyChordEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
        : ChordEngraver(libraryScope, pScoreMeter)
    {
    }

    inline ImoNote* my_get_min_note() { return get_min_note(); }
    inline ImoNote* my_get_max_note() { return get_max_note(); }
	inline ImoNote* my_get_base_note() { return get_base_note(); }
    inline bool my_is_stem_up() { return is_stem_up(); }
    inline std::list<ChordNoteData*>& my_get_notes() { return m_notes; }
    inline void my_decide_on_stem_direction() { decide_on_stem_direction(); }
    inline void my_layout_noteheads() { layout_noteheads(); }
    inline void my_align_noteheads() { align_noteheads(); }
    inline void my_arrange_notheads_to_avoid_collisions() { arrange_notheads_to_avoid_collisions(); }
    inline LUnits my_get_stem_width() { return m_stemWidth; }
    inline void my_add_stem_and_flag() { add_stem_and_flag(); }
    inline void my_set_anchor_offset() { set_anchor_offset(); }

};


//---------------------------------------------------------------------------------------
class ChordEngraverTestFixture
{
public:
    LibraryScope m_libraryScope;

    ChordEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pMeter(NULL)
        , m_pNoteEngrv(NULL)
        , m_pChord(NULL)
        , m_pNote1(NULL)
        , m_pNote2(NULL)
        , m_pNote3(NULL)
        , m_pShape1(NULL)
        , m_pShape2(NULL)
        , m_pShape3(NULL)
        , m_pStorage(NULL)
        , m_pChordEngrv(NULL)
    {
    }

    ~ChordEngraverTestFixture()    //TearDown fixture
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

    void create_chord(int step1, int octave1, int step2, int octave2,
                     int step3=-1, int octave3=-1, int noteType=k_eighth)
    {
        delete_chord();
        Document doc(m_libraryScope);
        m_pChord = static_cast<ImoChord*>( ImFactory::inject(k_imo_chord, &doc) );

        m_pNote1 = ImFactory::inject_note(&doc, step1, octave1, noteType, k_no_accidentals);
        m_pNote1->include_in_relation(&doc, m_pChord);

        m_pNote2 = ImFactory::inject_note(&doc, step2, octave2, noteType, k_no_accidentals);
        m_pNote2->include_in_relation(&doc, m_pChord);

        if (step3 >= 0)
        {
            m_pNote3 = ImFactory::inject_note(&doc, step3, octave3, noteType, k_no_accidentals);
            m_pNote3->include_in_relation(&doc, m_pChord);
        }
        else
            m_pNote3 = NULL;

        m_pMeter = new ScoreMeter(1, 1, 180.0f);
        m_pStorage = new ShapesStorage();
        m_pNoteEngrv = new NoteEngraver(m_libraryScope, m_pMeter, m_pStorage);
        m_pShape1 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote1, 0, 0, k_clef_G2, UPoint(10.0f, 15.0f)) );
        m_pShape2 =
            dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote2, 0, 0, k_clef_G2, UPoint(10.0f, 15.0f)) );
        if (step3 >= 0)
            m_pShape3 =
                dynamic_cast<GmoShapeNote*>(m_pNoteEngrv->create_shape(m_pNote3, 0, 0, k_clef_G2, UPoint(10.0f, 15.0f)) );
        else
            m_pShape3 = NULL;
    }

    void delete_chord()
    {
        delete m_pMeter;
        delete m_pNoteEngrv;
        delete m_pNote1;
        delete m_pNote2;
        delete m_pNote3;
        delete m_pShape1;
        delete m_pShape2;
        delete m_pShape3;
        delete m_pStorage;
        delete m_pChordEngrv;
    }

    void engrave_chord()
    {
        int iInstr = 0;
        int iStaff = 0;
        int iSystem = 0;
        int iCol = 0;
        UPoint pos(0.0f, 0.0f);

        //first note
        m_pChordEngrv = new MyChordEngraver(m_libraryScope, m_pMeter);
        m_pChordEngrv->set_start_staffobj(m_pChord, m_pNote1, m_pShape1, iInstr, iStaff,
                                          iSystem, iCol, pos);
        m_pStorage->save_engraver(m_pChordEngrv, m_pChord);

        if (m_pShape3)
        {
            //second note
            m_pChordEngrv->set_middle_staffobj(m_pChord, m_pNote2, m_pShape2, iInstr,
                                        iStaff, iSystem, iCol);
            //last note
            m_pChordEngrv->set_end_staffobj(m_pChord, m_pNote3, m_pShape3, iInstr,
                                     iStaff, iSystem, iCol);
        }
        else
            m_pChordEngrv->set_end_staffobj(m_pChord, m_pNote2, m_pShape2, iInstr,
                                     iStaff, iSystem, iCol);
    }


    ScoreMeter* m_pMeter;
    NoteEngraver* m_pNoteEngrv;
    ImoChord* m_pChord;
    ImoNote* m_pNote1;
    ImoNote* m_pNote2;
    ImoNote* m_pNote3;
    GmoShapeNote* m_pShape1;
    GmoShapeNote* m_pShape2;
    GmoShapeNote* m_pShape3;
    ShapesStorage* m_pStorage;
    MyChordEngraver* m_pChordEngrv;

};

SUITE(ChordEngraverTest)
{

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_CreateShapes)
    {
        create_chord(0,4,  2,4,  4,4);
        engrave_chord();

        m_pChordEngrv->create_shapes();
        MyChordEngraver* pEngrv = dynamic_cast<MyChordEngraver*>(m_pStorage->get_engraver(m_pChord));

        CHECK( pEngrv != NULL );
        CHECK( pEngrv->my_get_notes().size() == 3 );
        CHECK( pEngrv->my_get_base_note() == m_pNote1 );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NotesSortedByPitch)
    {
        create_chord(4,4,  0,4,  2,4);
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_get_base_note() == m_pNote1 );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->pNote == m_pNote2 );
        ++it;
        CHECK( (*it)->pNote == m_pNote3 );
        ++it;
        CHECK( (*it)->pNote == m_pNote1 );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleA1)
    {
        create_chord(5,4,  1,5);     //(a4,d5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleA2)
    {
        create_chord(2,4,  0,5);     //(e4,c5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleA3)
    {
        create_chord(4,4,  1,5);     //(g4.d5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleB3Up)
    {
        create_chord(2,4,  4,4,  3,5);     //(e4, g4, f5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_StemDirectionRuleB3Down)
    {
        create_chord(2,4,  1,5,  3,5);     //(e4, d5, f5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemDown)
    {
        create_chord(5,4,  6,4,  1,5);     //(a4, b4, d5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemDown2)
    {
        create_chord(5,4,  6,4,  0,5);     //(a4, b4, c5)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == false );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemUp)
    {
        create_chord(3,4,  4,4,  6,4);     //(f4, g4, b4)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_NoteheadReversedStemUp2)
    {
        create_chord(3,4,  4,4,  5,4);     //(f4, g4, a4)
        engrave_chord();
        m_pChordEngrv->create_shapes();

        CHECK( m_pChordEngrv->my_is_stem_up() == true );
        std::list<ChordEngraver::ChordNoteData*>& notes = m_pChordEngrv->my_get_notes();
        std::list<ChordEngraver::ChordNoteData*>::iterator it = notes.begin();
        CHECK( (*it)->fNoteheadReversed == false );
        ++it;
        CHECK( (*it)->fNoteheadReversed == true );
        ++it;
        CHECK( (*it)->fNoteheadReversed == false );

        delete_chord();
    }


    // Engrave chord. Case 1:  stem up, no accidentals, no reversed note ----------------

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AligneNoteheads_C1)
    {
        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left() << endl;

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_ArrangeNoteheads_C1)
    {
        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AddStemAndFlag_C1)
    {
        // adding stem doesn't alter positions

        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        m_pChordEngrv->my_add_stem_and_flag();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_SetOffset_C1)
    {
        // set offset doesn' alter positions

        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();
        m_pChordEngrv->my_add_stem_and_flag();

        m_pChordEngrv->my_set_anchor_offset();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_EngraveChord_C1)
    {
        create_chord(2,4,  4,4);     //(e4,g4)
        engrave_chord();

        m_pChordEngrv->create_shapes();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        CHECK ( is_equal(m_pShape1->get_stem_left(), 220.4f) );
        CHECK ( is_equal(m_pShape1->get_stem_width(), 21.6f) );

        delete_chord();
    }


    // Engrave chord. Case 2:  stem up, no accidentals, reversed note -------------------

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AligneNoteheads_C2)
    {
        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left() << endl;

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_ArrangeNoteheads_C2)
    {
        // reversed notehead is shifted: left = 10 + 232 - 21.6

        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();

        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 220.4f) );
        CHECK ( is_equal(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_AddStemAndFlag_C2)
    {
        // adding stem doesn't alter positions

        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();

        m_pChordEngrv->my_add_stem_and_flag();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_left(), 220.4f) );
        CHECK ( is_equal(m_pChordEngrv->my_get_stem_width(), 21.6f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_SetOffset_C2)
    {
        // set offset doesn' alter positions

        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();
        m_pChordEngrv->my_decide_on_stem_direction();
        m_pChordEngrv->my_align_noteheads();
        m_pChordEngrv->my_arrange_notheads_to_avoid_collisions();
        m_pChordEngrv->my_add_stem_and_flag();

        m_pChordEngrv->my_set_anchor_offset();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 220.4f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 210.4f) );
        CHECK ( is_equal(m_pShape2->get_left(), 220.4f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_EngraveChord_C2)
    {
        create_chord(2,4,  3,4);     //(e4,f4)
        engrave_chord();

        m_pChordEngrv->create_shapes();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 232.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 220.4f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 210.4f) );
        CHECK ( is_equal(m_pShape2->get_left(), 220.4f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 232.0f) );

        CHECK ( is_equal(m_pShape1->get_stem_left(), 220.4f) );
        CHECK ( is_equal(m_pShape1->get_stem_width(), 21.6f) );

        delete_chord();
    }

    TEST_FIXTURE(ChordEngraverTestFixture, ChordEngraver_EngraveChord_Test00030)
    {
        create_chord(0,4,  2,4,  4,4,  k_whole);     //(c4,e4,g4)
        engrave_chord();

        m_pChordEngrv->create_shapes();

        CHECK ( is_equal(m_pShape1->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape1->get_notehead_width(), 366.0f) );

        CHECK ( is_equal(m_pShape2->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape2->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape2->get_notehead_width(), 366.0f) );

        CHECK ( is_equal(m_pShape3->get_notehead_left(), 10.0f) );
        CHECK ( is_equal(m_pShape3->get_anchor_offset(), 0.0f) );
        CHECK ( is_equal(m_pShape3->get_left(), 10.0f) );
        CHECK ( is_equal(m_pShape3->get_notehead_width(), 366.0f) );

        CHECK ( is_equal(m_pShape1->get_stem_left(), 0.0f) );
        CHECK ( is_equal(m_pShape1->get_stem_width(), 0.0f) );

        //cout << "note1. left=" << m_pShape1->get_notehead_left()
        //     << ", width=" << m_pShape1->get_notehead_width() << endl;
        //cout << "note2. left=" << m_pShape2->get_notehead_left()
        //     << ", width=" << m_pShape2->get_notehead_width() << endl;
        //cout << "note3. left=" << m_pShape3->get_notehead_left()
        //     << ", width=" << m_pShape3->get_notehead_width() << endl;
        //cout << "stem. left=" << m_pShape1->get_stem_left()
        //     << ", width=" << m_pShape1->get_stem_width() << endl;

        delete_chord();
    }

}

