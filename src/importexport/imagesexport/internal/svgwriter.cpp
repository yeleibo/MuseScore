/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <typeinfo>
#include "svgwriter.h"

#include "svggenerator.h"

#include "libmscore/masterscore.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/stafflines.h"
#include "libmscore/tempo.h"
#include "svggenerator.h"
#include "log.h"
#include <qglobal.h>
#include <QtCore/qthread.h>


using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::io;

struct ExportDot
{
    std::string noteDotSvgId;
};
struct ExportNote
{
    int noteValue;
    Ms::TDuration durationType;
    int segmentIndex;
    int measureIndex;
    int systemIndex;
    int trackIndex;
    int staffIndex;
    int noteIndex;
    std::string svgId;
    std::vector<ExportDot> dots;
    float startTime;
    float endTime;
    double startX;
    double startY;
    double width;
    double height;

    std::string toJson() {
        
        QString result = QObject::tr(
            "{\"systemIndex\":%1,\"measureList1\":%2,\"staffEntries\":%3,\"measureList2\":%4,\"key\":%5,\"graphicalVoiceEntries\":%6,\"notes\":%7,\"startTime\":%8,\"endTime\":%9,\"svgId\":\"%10\"}"
         ).arg(QString::number(systemIndex), QString::number(measureIndex), QString::number(segmentIndex), QString::number(staffIndex), QString::number(noteValue), QString::number(trackIndex), QString::number(noteIndex), QString::number(int(startTime*1000)), QString::number(int(endTime*1000)), QString::fromStdString(svgId));
        
        return result.toStdString();
    }
    std::string toJson2() {

        QString result = QObject::tr(
            "{\"systemIndex\":%1,\"measureIndex\":%2,\"segmentIndex\":%3,\"staffIndex\":%4,\"noteValue\":%5,\"trackIndex\":%6,\"noteIndex\":%7,\"startTime\":%8,\"endTime\":%9,\"svgId\":\"%10\",\"startX\":%11,\"startY\":%12,\"width\":%13,\"height\":%14}"
        ).arg(QString::number(systemIndex), QString::number(measureIndex), QString::number(segmentIndex), QString::number(staffIndex), QString::number(noteValue), QString::number(trackIndex), QString::number(noteIndex), QString::number(int(startTime * 1000)), QString::number(int(endTime * 1000)), QString::fromStdString(svgId), QString::number(startX), QString::number(startY), QString::number(width), QString::number(height));

        return result.toStdString();
    }
};
struct ExportSystem
{
    int systemIndex;
    double startX;
    double startY;
    double width;
    double height;

    std::string toJson() {

        QString result = QObject::tr(
            "{\"startX\":%1,\"startY\":%2,\"width\":%3,\"height\":%4,\"systemIndex\":%5}"
        ).arg(QString::number(startX), QString::number(startY), QString::number(width), QString::number(height), QString::number(systemIndex));

        return result.toStdString();
    }
};
struct ExportMeasure
{
    int systemIndex;
    int measureIndex;
    double startX;
    double startY;
    double width;
    double height;

    std::string toJson() {

        QString result = QObject::tr(
            "{\"startX\":%1,\"startY\":%2,\"width\":%3,\"height\":%4,\"measureIndex\":%5,\"systemIndex\":%6}"
        ).arg(QString::number(startX), QString::number(startY), QString::number(width), QString::number(height), QString::number(measureIndex), QString::number(systemIndex));
    
        return result.toStdString();
    }

};

struct ExportSegment
{
    int measureIndex;
    int systemIndex;
    int segmentIndex;
    double beatNum;
    double startX;
    double startY;
    double width;
    double height;


    std::string toJson() {


        QString result = QObject::tr(
            "{\"startX\":%1,\"startY\":%2,\"width\":%3,\"height\":%4,\"beatNum\":%5,\"systemIndex\":%6,\"measureIndex\":%7,\"segmentIndex\":%8}"
        ).arg(QString::number(startX), QString::number(startY), QString::number(width), QString::number(height), QString::number(beatNum), QString::number(systemIndex), QString::number(measureIndex), QString::number(segmentIndex));

        return result.toStdString();
    }

};

struct ExportSheetMusicJson
{
    std::vector<ExportNote> notes;
    std::vector<ExportSystem> systems;
    std::vector<ExportMeasure> measures;
    std::vector<ExportSegment> segments;
    Ms::TimeSigFrac timeSig;
    qreal tempo;
    std::string toJson() {
        std::string str = "{";
     

        str += "\"notes\":[";
        for (std::vector<ExportNote>::iterator note = notes.begin(); note != notes.end(); note++) {
            str += (*note).toJson();
            if (note != notes.end()-1) {
                str += ",";
            }
            
            
        }

        str += "]}";
        return str;
    }
    std::string toJson2() {
        std::string str = "{";

        str += "\"systems\":[";
        for (std::vector<ExportSystem>::iterator system = systems.begin(); system != systems.end(); system++) {
            str += (*system).toJson();
            if (system != systems.end() - 1) {
                str += ",";
            }
        }
        str += "],";

        str += "\"measures\":[";
        for (std::vector<ExportMeasure>::iterator measure = measures.begin(); measure != measures.end(); measure++) {
            str += (*measure).toJson();
            if (measure != measures.end() - 1) {
                str += ",";
            }
        }
        str += "],";

        str += "\"segments\":[";
        for (std::vector<ExportSegment>::iterator segment = segments.begin(); segment != segments.end(); segment++) {
            str += (*segment).toJson();
            if (segment != segments.end() - 1) {
                str += ",";
            }
        }
        str += "],";

        str += "\"notes\":[";
        for (std::vector<ExportNote>::iterator note = notes.begin(); note != notes.end(); note++) {
            str += (*note).toJson2();
            if (note != notes.end() - 1) {
                str += ",";
            }


        }

        str += "]}";
        return str;
    }
};

std::pair<int, float> barbeat(const Element* element) 
{
    if (!element) {
        return std::pair<int, float>(0, 0.0F);
    }

    const Element* parent = element;
    while (parent && parent->type() != ElementType::SEGMENT && parent->type() != ElementType::MEASURE) {
        parent = parent->parent();
    }

    if (!parent) {
        return std::pair<int, float>(0, 0.0F);
    }

    int bar = 0;
    int beat = 0;
    int ticks = 0;

    const Ms::TimeSigMap* timeSigMap = element->score()->sigmap();
    int ticksB = Ms::ticks_beat(timeSigMap->timesig(0).timesig().denominator());

    if (parent->type() == ElementType::SEGMENT) {
        const Ms::Segment* segment = static_cast<const Ms::Segment*>(parent);
        timeSigMap->tickValues(segment->tick().ticks(), &bar, &beat, &ticks);
        ticksB = Ms::ticks_beat(timeSigMap->timesig(segment->tick().ticks()).timesig().denominator());
    }
    else if (parent->type() == ElementType::MEASURE) {
        const Measure* measure = static_cast<const Measure*>(parent);
        bar = measure->no();
        beat = -1;
        ticks = 0;
    }

    return std::pair<int, float>(bar + 1, beat + 1 + ticks / static_cast<float>(ticksB));
}


ExportNote getNoteSvgInfoByParent(Ms::Rest* myRest) {
    ExportNote exportNote = ExportNote();
    exportNote.noteValue = 0;
    exportNote.staffIndex = myRest->staffIdx();
    exportNote.durationType = myRest->actualDurationType();
    exportNote.measureIndex = barbeat(myRest).first-1;

   QString name= exportNote.durationType.name();
    Ms::Element* parent = myRest->parent();
    Ms::Segment* segment;
    while (parent->type() != Ms::ElementType::PAGE)
    {

        if (parent->type() == Ms::ElementType::SEGMENT) {
            segment = static_cast<Ms::Segment*>(parent);
            Measure* measure = static_cast<Measure*>(parent->parent());
            Ms::SegmentList segments = measure->segments();
            Ms::Segment* segmentTemp = segments.first();
            int segmentIndex = -1;
            for (int j = 0; j < segments.size(); j++) {
                Ms::SegmentType type = segmentTemp->segmentType();
                if (type == Ms::SegmentType::ChordRest) {
                    segmentIndex++;
                }
                if (segment == segmentTemp) {
                    exportNote.segmentIndex = segmentIndex;
                    break;
                }
                segmentTemp = segmentTemp->next();
            }




        }



        if (parent->type() == Ms::ElementType::SYSTEM) {
            Page* page = static_cast<Page*>(parent->parent());

            Ms::System* system = static_cast<Ms::System*>(parent);
            QList<Ms::System*> systems = page->systems();
            int systeIndex = -1;
            for (int i = 0;i< systems.size(); i++) {
                if (systems[i]->staves()->size() == 0) {
                    continue;
                }
                systeIndex++;
                if (systems[i] == system) {
                    exportNote.systemIndex = systeIndex;
                }
            }
        }


        parent = parent->parent();
    }
    return exportNote;
}




/// <summary>
/// 获取note的输出信息
/// </summary>
/// <param name="myNote"></param>
/// <returns></returns>
ExportNote getNoteSvgInfoByParent(Ms::Note* myNote) {
    //升八度或者降八度
    int uprodown = myNote->ottaveCapoFret();
    ExportNote exportNote = ExportNote();
    exportNote.noteValue = myNote->pitch()+ uprodown;
    exportNote.staffIndex = myNote->staffIdx();
    exportNote.trackIndex = myNote->track();
    Ms::Element* parent=myNote->parent();
    exportNote.measureIndex = barbeat(myNote).first-1;

    Ms::Segment* segment;
    while (parent->type() != Ms::ElementType::PAGE)
    {

        if (parent->type() == Ms::ElementType::SEGMENT) {
            segment = static_cast<Ms::Segment*>(parent);
            Measure* measure = static_cast<Measure*>(parent->parent());
            Ms::SegmentList segments = measure->segments();
            Ms::Segment* segmentTemp = segments.first();
            int segmentIndex = -1;
            for (int j = 0; j < segments.size(); j++) {
                Ms::SegmentType type = segmentTemp->segmentType();
                if (type == Ms::SegmentType::ChordRest) {
                    segmentIndex++;
                }
                if (segment == segmentTemp) {
                    exportNote.segmentIndex = segmentIndex;
                    //0-3是第一staff即右手,4-7是第二staff即左手,m的含义是第几声部
               
                 
                        

                                Ms::ChordRest* cr = segment->cr(exportNote.trackIndex);
                                if (cr != NULL && cr->type()== Ms::ElementType::CHORD) {

                                    /* QVariant qv = QVariant::fromValue("abc");
                                     cr->setProperty(Ms::Pid::SUBTYPE, qv);*/
                                    Ms::Chord* chord = static_cast<Ms::Chord*>(cr);
                                   exportNote.durationType= chord->actualDurationType();
                     
                                   QString name = exportNote.durationType.name();
                                    std::vector<Note*> notes = chord->notes();
                                
                                    for (int t = 0; t < notes.size(); t++) {
                                        if (notes[t] == myNote)
                                        {  
                                
                                            exportNote.noteIndex = t;
                                         
                                        }


                                    }
                                }
                        
                    
                    break;
                }
                segmentTemp = segmentTemp->next();
            }


     

        }

        if (parent->type() == Ms::ElementType::SYSTEM) {
            Page* page = static_cast<Page*>(parent->parent());

            Ms::System* system = static_cast<Ms::System*>(parent);
            QList<Ms::System*> systems= page->systems();
            int systeIndex = -1;
            for (int i = 0; i < systems.size(); i++) {
                if (systems[i]->staves()->size() == 0) {
                    continue;
                }
                systeIndex++;
                if (systems[i] == system) {
                    exportNote.systemIndex= systeIndex;
                }
            }
            
        }


        parent = parent->parent();
    }
    return exportNote;
}


std::vector<INotationWriter::UnitType> SvgWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PAGE };
}

mu::Ret SvgWriter::write(INotationPtr notation, Device& destinationDevice, const Options& options)
{

    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }
    Ms::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }
    ExportSheetMusicJson exportSheetMusicJson = ExportSheetMusicJson();
    Ms::MasterScore* masterScore = static_cast<Ms::MasterScore*>(score);

    Ms::EventMap events;
    score->renderMidi(&events, false, false, score->synthesizerState());
    //节拍相关
    Ms::TempoMap*  tempo=masterScore->tempomap();
    std::map<int, Ms::TEvent>::iterator tEvent = tempo->begin();
    exportSheetMusicJson.tempo=(*tEvent).second.tempo;

   // 节拍相关  
    Ms::TimeSigMap*  sigmap = masterScore->sigmap();

    std::map<int, Ms::SigEvent>::iterator sigEvent = sigmap->begin();
    exportSheetMusicJson.timeSig=(*sigEvent).second.timesig();
    score->setPrinting(true); // don’t print page break symbols etc.

    Ms::MScore::pdfPrinting = true;
    Ms::MScore::svgPrinting = true;

    const QList<Ms::Page*>& pages = score->pages();
    double pixelRationBackup = Ms::MScore::pixelRatio;

    const int PAGE_NUMBER = options.value(OptionKey::PAGE_NUMBER, Val(0)).toInt();
    if (PAGE_NUMBER < 0 || PAGE_NUMBER >= pages.size()) {
        return false;
    }

    Ms::Page* page = pages.at(PAGE_NUMBER);

    
    SvgGenerator printer;
    QString title(score->title());
    printer.setTitle(pages.size() > 1 ? QString("%1 (%2)").arg(title).arg(PAGE_NUMBER + 1) : title);
    printer.setOutputDevice(&destinationDevice);

    const int TRIM_MARGINS_SIZE = configuration()->trimMarginPixelSize();

    RectF pageRect = page->abbox();
    if (TRIM_MARGINS_SIZE >= 0) {
        QMarginsF margins(TRIM_MARGINS_SIZE, TRIM_MARGINS_SIZE, TRIM_MARGINS_SIZE, TRIM_MARGINS_SIZE);
        QRectF t= page->tbbox().toQRectF();
        pageRect = RectF::fromQRectF(page->tbbox().toQRectF() + margins);
    }

    qreal width = pageRect.width();
    qreal height = pageRect.height();
    printer.setSize(QSize(width, height));
    printer.setViewBox(QRectF(0, 0, width, height));

    mu::draw::Painter painter(&printer, "svgwriter");
    painter.setAntialiasing(true);
    if (TRIM_MARGINS_SIZE >= 0) {
        painter.translate(-pageRect.topLeft());
    }

    Ms::MScore::pixelRatio = Ms::DPI / printer.logicalDpiX();

    if (!options[OptionKey::TRANSPARENT_BACKGROUND].toBool()) {
        painter.fillRect(pageRect, engravingConfiguration()->whiteColor());
    }

    // 1st pass: StaffLines
    for (const Ms::System* system : page->systems()) {
        int stavesCount = system->staves()->size();

        std::vector<Ms::MeasureBase*> measures = system->measures();
      /*      for (int i = 0; i < measures.size(); i++) {
                Ms::MeasureBase* measure = measures[i];
                bool   measure->hasStaff();
                Staff* staff=measure->staff();
            }*/

        for (int staffIndex = 0; staffIndex < stavesCount; ++staffIndex) {
            if (score->staff(staffIndex)->invisible(Ms::Fraction(0, 1)) || !score->staff(staffIndex)->show()) {
                continue; // ignore invisible staves
            }

            if (system->staves()->isEmpty() || !system->staff(staffIndex)->show()) {
                continue;
            }

            Ms::Measure* firstMeasure = system->firstMeasure();
            if (!firstMeasure) { // only boxes, hence no staff lines
                continue;
            }

            // The goal here is to draw SVG staff lines more efficiently.
            // MuseScore draws staff lines by measure, but for SVG they can
            // generally be drawn once for each system. This makes a big
            // difference for scores that scroll horizontally on a single
            // page. But there are exceptions to this rule:
            //
            //   ~ One (or more) invisible measure(s) in a system/staff ~
            //   ~ One (or more) elements of type HBOX or VBOX          ~
            //
            // In these cases the SVG staff lines for the system/staff
            // are drawn by measure.
            //
            bool byMeasure = true;
       /*     for (Ms::MeasureBase* measure = firstMeasure; measure; measure = system->nextMeasure(measure)) {
                if (!measure->isMeasure() || !Ms::toMeasure(measure)->visible(staffIndex)) {
                    byMeasure = true;
                    break;
                }
            }*/

            if (byMeasure) {     // Draw visible staff lines by measure
                for (Ms::MeasureBase* measure = firstMeasure; measure; measure = system->nextMeasure(measure)) {
                    if (measure->isMeasure() && Ms::toMeasure(measure)->visible(staffIndex)) {
                        Ms::StaffLines* sl = Ms::toMeasure(measure)->staffLines(staffIndex);
                        printer.setElement(sl);
                        Ms::paintElement(painter, sl);
                    }
                }
            } else {   // Draw staff lines once per system
                Ms::StaffLines* firstSL = system->firstMeasure()->staffLines(staffIndex)->clone();
                Ms::StaffLines* lastSL =  system->lastMeasure()->staffLines(staffIndex);

                qreal lastX =  lastSL->bbox().right()
                              + lastSL->pagePos().x()
                              - firstSL->pagePos().x();
                std::vector<mu::LineF>& lines = firstSL->getLines();
                for (size_t l = 0, c = lines.size(); l < c; l++) {
                    lines[l].setP2(mu::PointF(lastX, lines[l].p2().y()));
                }
                printer.setElement(firstSL);
                Ms::paintElement(painter, firstSL);
            
            }
        }
    }
  
    // 2nd pass: the rest of the elements
    QList<Ms::Element*> elements = page->elements();
    std::stable_sort(elements.begin(), elements.end(), Ms::elementLessThan);

    int lastNoteIndex = -1;
    for (int i = 0; i < PAGE_NUMBER; ++i) {
        for (const Ms::Element* element: pages[i]->elements()) {
            if (element->type() == Ms::ElementType::NOTE) {
                lastNoteIndex++;
            }
        }
    }

    NotesColors notesColors = parseNotesColors(options.value(OptionKey::NOTES_COLORS, Val()).toQVariant());

    QList<Ms::Note*> notes;
    for (Ms::Element* element : elements) {
        // Always exclude invisible elements
        if (!element->visible()) {
            continue;
        }

        Ms::ElementType type = element->type();
        switch (type) { // In future sub-type code, this switch() grows, and eType gets used
        case Ms::ElementType::STAFF_LINES: // Handled in the 1st pass above
            continue; // Exclude from 2nd pass
            break;
        default:
            break;
        }

        // Set the Element pointer inside SvgGenerator/SvgPaintEngine
        printer.setElement(element);

        std::map<QString, QString> attrabute = std::map<QString, QString>();
        if (element->hasStaff()) {
      
                int measureIndex = barbeat(element).first - 1;
                int staffIndex = element->staffIdx();
                QString staffAttabute = QString::number(measureIndex) + "-" + QString::number(staffIndex);
                attrabute["staff"] = staffAttabute;
                
            
        }
        // Paint it
        if (element->type() == Ms::ElementType::NOTE) {
            Ms::Note* note = static_cast<Ms::Note*>(element);
 
            notes.append(note);
            if (!notesColors.isEmpty()) {
                 QColor color = element->color().toQColor();
                int currentNoteIndex = (++lastNoteIndex);

                if (notesColors.contains(currentNoteIndex)) {
                    color = notesColors[currentNoteIndex];
                }
                element->setColor(color);
            }



            ExportNote noteInfo= getNoteSvgInfoByParent(note);
            bool isStartHaveValue = false;
            for (auto i = events.begin(); i != events.end(); ++i) {
                  const Ms::NPlayEvent& event = i->second;
                if (event.note() == note) {
                    float time = float(i->first) / Ms::MScore::division;
             
                    if (!isStartHaveValue) {
                        noteInfo.startTime = time;
                        isStartHaveValue = true;
                    }else {
                        if (noteInfo.startTime > time) {
                            noteInfo.endTime = noteInfo.startTime;
                            noteInfo.startTime = time;
                        }
                        else {
                                noteInfo.endTime = time;
                        }
                    
                    }
                }
            }

            QString svgId = "note-" + QString::number(noteInfo.systemIndex) + "-" + QString::number(noteInfo.measureIndex) + "-" + QString::number(noteInfo.segmentIndex) + "-" + QString::number(noteInfo.trackIndex) + "-" + QString::number(noteInfo.staffIndex) + "-" + QString::number(noteInfo.noteValue)+"-"+ QString::number(noteInfo.startTime) + "-" + QString::number(noteInfo.endTime);
            attrabute["id"] = svgId;
              
            noteInfo.svgId = svgId.toStdString();

            printer.setElement(note);
            PointF elementPosition(note->pagePos());
            painter.translate(elementPosition);
            mu::draw::Painter::State  state = painter.state();
            noteInfo.startX = state.transform.m31();
            noteInfo.startY = state.transform.m32();
            noteInfo.width = note->width();
            noteInfo.height = note->height();
            painter.translate(-elementPosition);

            exportSheetMusicJson.notes.push_back(noteInfo);
            
            ////添加符点音符信息
            //QVector<Ms::NoteDot*> noteDots = note->dots();
            //if (!noteDots.isEmpty()) {
            //    for (int i = 0; i < noteDots.size(); i++) {
            //        Ms::NoteDot* noteDot = noteDots[i];
            //        QString noteDotSvgId = "noteDot-" + QString::number(noteInfo.systemIndex) + "-" + QString::number(noteInfo.measureIndex) + "-" + QString::number(noteInfo.segmentIndex) + "-" + QString::number(noteInfo.trackIndex) + "-" + QString::number(noteInfo.staffIndex)  + "-" + QString::number(i);
            //     
            //        attrabute["id"] = noteDotSvgId;
            //  
            //        noteInfo.dots.push_back(ExportDot({ noteDotSvgId.toStdString() }));
            //    }
            //}

        }
        else if (element->type() == Ms::ElementType::REST) {
            Ms::Rest* rest = static_cast<Ms::Rest*>(element);
            ExportNote noteInfo = getNoteSvgInfoByParent(rest);
 
            //休止符
            printf("休止符");
        }
        else if (element->type() == Ms::ElementType::BAR_LINE) {
                
                QString svgId = "svgId" + QString::number(barbeat(element).first - 1);
                attrabute["id"] = svgId;
            

        }
        else if (element->type() == Ms::ElementType::BEAM) {
                Ms::Beam* beam = static_cast<Ms::Beam*>(element);
                QVector<ChordRest*> cChordRest = beam->elements();
                int measureIndex = barbeat(cChordRest.first()).first - 1;
                int staffIndex = cChordRest.first()->staffIdx();
                QString staffAttabute = QString::number(measureIndex) + "-" + QString::number(staffIndex);
                attrabute["staff"] = staffAttabute;
            
        }
        
        
        element->setAttrabute(attrabute);
        Ms::paintElement(painter, element);
        attrabute.clear();
    }
    std::stable_sort(notes.begin(), notes.end(), Ms::elementLessThan);
    QList<Ms::System*> systems= page->systems();
    int systeIndex = -1;
    for (int i = 0; i < systems.size(); i++) {
        Ms::System* system = systems[i];
        if (system->staves()->size() == 0) {
            continue ;
        }
        ExportSystem exportSystem = ExportSystem();
        systeIndex++;
        exportSystem.systemIndex = systeIndex;
        //获取system位置信息
        printer.setElement(system);
        PointF elementPosition(system->pagePos());
        painter.translate(elementPosition);
        mu::draw::Painter::State  state = painter.state();
        exportSystem.startX = state.transform.m31();
        exportSystem.startY = state.transform.m32();
        exportSystem.width = system->width();
        exportSystem.height = system->height();
        exportSheetMusicJson.systems.push_back(exportSystem);
        painter.translate(-elementPosition);


        std::vector<Ms::MeasureBase*> measures=system->measures();
        for (int j = 0; j < measures.size(); j++) {
            //获取measure位置信息
            Ms::Measure* measure = static_cast<Ms::Measure*>(measures[j]);
            ExportMeasure exportMeasure = ExportMeasure();
            exportMeasure.measureIndex = measure->no();
            exportMeasure.systemIndex = exportSystem.systemIndex;
            printer.setElement(measure);
            PointF elementPosition(measure->pagePos());
            painter.translate(elementPosition);
            mu::draw::Painter::State  state = painter.state();
            exportMeasure.startX = state.transform.m31();
            exportMeasure.startY = state.transform.m32();
            exportMeasure.width = measure->width();
            exportMeasure.height = measure->height();
            exportSheetMusicJson.measures.push_back(exportMeasure);
            painter.translate(-elementPosition);
            
            Ms::SegmentList segments = measure->segments();
            Ms::Segment* segment = segments.first();
            int segmentIndex = -1;
            for (int j = 0; j < segments.size(); j++) {
                ///只导出音符的segment
                if (segment->segmentType() == Ms::SegmentType::ChordRest) {
                   ExportSegment exportSegment = ExportSegment();
                   segmentIndex++;
                   exportSegment.segmentIndex = segmentIndex;
                   exportSegment.measureIndex = exportMeasure.measureIndex;
                   exportSegment.systemIndex = exportSystem.systemIndex;
                    Fraction beat = segment->beat();
                    exportSegment.beatNum=double(beat.numerator())/beat.denominator();
                    printer.setElement(segment);
                    PointF elementPosition(segment->pagePos());
                    painter.translate(elementPosition);
                    mu::draw::Painter::State  state = painter.state();
                    exportSegment.startX = state.transform.m31();
                    exportSegment.startY = state.transform.m32();
                    exportSegment.width = segment->width();
                    exportSegment.height = segment->height();
                 
                    exportSheetMusicJson.segments.push_back(exportSegment);
                    painter.translate(-elementPosition);
                }
             
                segment = segment->next();
            }
        
        }
    }  
 
    painter.endDraw(); // Writes MuseScore SVG file to disk, finally

    // Clean up and return
    Ms::MScore::pixelRatio = pixelRationBackup;
    score->setPrinting(false);
    Ms::MScore::pdfPrinting = false;
    Ms::MScore::svgPrinting = false;





   //tojson  
    std::string jsonStr = exportSheetMusicJson.toJson();
    QString importFilePath=   score->importedFilePath();
    int lastDot = importFilePath.lastIndexOf(".");
    QString jsonFilePath = importFilePath.mid(0, lastDot) + ".txt";
    QFile jsonFile = QFile(jsonFilePath);

    if (jsonFile.exists())
    {
        jsonFile.remove();
    }
    jsonFile.open(QIODevice::WriteOnly);
    
    jsonFile.write(jsonStr.c_str());
    jsonFile.close();
    return true;
}


SvgWriter::NotesColors SvgWriter::parseNotesColors(const QVariant& obj) const
{
    QVariantMap map = obj.toMap();
    NotesColors result;

    for (const QString& noteNumber : map.keys()) {
        result[noteNumber.toInt()] = map[noteNumber].value<QColor>();
    }

    return result;
}

